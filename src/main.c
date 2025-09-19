/*
Tehtävien pisteet
Viikkotehtävä 2, 3p.
LEDit kiertävät eri värit punainen -> keltainen -> vihreä -> punainen jne. Toteutus tehty taskien kautta käyttäen tilakonetta.
Napille tehty keskeytystoiminto, jolla led tila otetaan talteen, ja uudelleen painamalla tilojen kiertoa jatketaan.
Napeille 2-4 lisätty päälle/pois toiminto, nappi 5 käynnistää keltaisen välyttämisen.

Viikkotehtävä 3, 1p.
UARTin kautta lähettämällä R,G,Y voi väläyttää LEDiä, LEDien tilakoneet on vaihdettu toimimaan dispatcherin signaalien mukaan.
FIFO puskuriin voi syöttää "RGYRGY" merkkijono, jonka mukaan LEDejä väläytetään syötetyssä sekvenssissä.
*/

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>
#include <zephyr/drivers/uart.h>

// configure buttons
#define BUTTON_0 DT_ALIAS(sw0)
#define BUTTON_1 DT_ALIAS(sw1)
#define BUTTON_2 DT_ALIAS(sw2)
#define BUTTON_3 DT_ALIAS(sw3)
#define BUTTON_4 DT_ALIAS(sw4)

// Led and button pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(BUTTON_1, gpios, {0});
static const struct gpio_dt_spec button_2 = GPIO_DT_SPEC_GET_OR(BUTTON_2, gpios, {0});
static const struct gpio_dt_spec button_3 = GPIO_DT_SPEC_GET_OR(BUTTON_3, gpios, {0});
static const struct gpio_dt_spec button_4 = GPIO_DT_SPEC_GET_OR(BUTTON_4, gpios, {0});
static struct gpio_callback button_0_data, button_1_data, button_2_data, button_3_data, button_4_data;

// led thread initialization
#define STACKSIZE 500
#define PRIORITY 5

void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void blue_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);
void yellow_blink_task(void *, void *, void*);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(blue_thread,STACKSIZE,blue_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_blink_thread,STACKSIZE,yellow_blink_task,NULL,NULL,NULL,PRIORITY,0,0);

// UART initialization
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

// Create dispatcher FIFO buffer
K_FIFO_DEFINE(dispatcher_fifo);

// semaphori ohjausta varten
struct k_sem release_sem;
K_SEM_DEFINE(release_sem, 0, 1); // Init value = 0

// Condition variablet ja mutexit joka valolle
K_MUTEX_DEFINE(red_mutex);
K_CONDVAR_DEFINE(red_cv);

K_MUTEX_DEFINE(green_mutex);
K_CONDVAR_DEFINE(green_cv);

K_MUTEX_DEFINE(yellow_mutex);
K_CONDVAR_DEFINE(yellow_cv);

int r_manual, y_manual, g_manual = 0;
int led_state, led_yellow_state;
int prev_led_state;

// FIFO dispatcher data type
struct data_t {
	void *fifo_reserved;
	char msg[20];
	char color;
};

// Button interrupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Pause button pressed\n");
	if(led_state != 5){
		//prev_led_state = led_state;
		//led state 5 = pause state
		led_state = 5;
		k_thread_suspend(red_thread);
		k_thread_suspend(green_thread);
		k_thread_suspend(yellow_thread);
		k_thread_suspend(yellow_blink_thread);
	} else{
		led_state = 1;
		k_thread_resume(red_thread);
		k_thread_resume(green_thread);
		k_thread_resume(yellow_thread);
		k_thread_resume(yellow_blink_thread);
	}
}
void button_1_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	// red button
	//refactor button to use fifo
	char merkki = 'R';
	struct data_t *sendR = k_malloc(sizeof(struct data_t));
	sendR->color = merkki;
	k_fifo_put(&dispatcher_fifo, sendR);
}
void button_2_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	// yellow button
	char merkki = 'Y';
	struct data_t *sendY = k_malloc(sizeof(struct data_t));
	sendY->color = merkki;
	k_fifo_put(&dispatcher_fifo, sendY);
}
void button_3_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	// green button
	char merkki = 'G';
	struct data_t *sendG = k_malloc(sizeof(struct data_t));
	sendG->color = merkki;
	k_fifo_put(&dispatcher_fifo, sendG);
}
void button_4_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins){
	//yellow blink
	if (led_yellow_state == 0) {
		gpio_pin_set_dt(&red, 0);
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&blue, 0);

		k_thread_suspend(red_thread);
		k_thread_suspend(green_thread);
		k_thread_suspend(yellow_thread);
		led_yellow_state = 1;
		k_thread_resume(yellow_blink_thread);
		
	} else {
		
		gpio_pin_set_dt(&red, 0);
		gpio_pin_set_dt(&green, 0);
		gpio_pin_set_dt(&blue, 0);

		k_thread_suspend(yellow_blink_thread);
		k_thread_resume(red_thread);
		k_thread_resume(green_thread);
		k_thread_resume(yellow_thread);
		led_yellow_state = 0;
	}
}
// UART initialization
int init_uart(void) {
	if (!device_is_ready(uart_dev)) {
		return 1;
	} 
	return 0;
}

// Initialize leds
int  init_led() {
	
	// Led pin initialization
	int ret = gpio_pin_configure_dt(&red, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printk("Error:Red Led configure failed\n");		
		return ret;
	}
	ret = gpio_pin_configure_dt(&green, GPIO_OUTPUT_ACTIVE);	
	if (ret < 0) {
		printk("Error:Green Led configure failed\n");		
		return ret;
	}
	ret = gpio_pin_configure_dt(&blue, GPIO_OUTPUT_ACTIVE);	
	if (ret < 0) {
		printk("Error:Blue Led configure failed\n");		
		return ret;
	}

	// set led off
	gpio_pin_set_dt(&red,0);
	gpio_pin_set_dt(&green,0);
	gpio_pin_set_dt(&blue,0);
	printk("Led initialized ok\n");
	led_state = 0;
	led_yellow_state = 0;
	k_thread_suspend(yellow_blink_thread);
	return 0;
}

// Button initialization
int init_button() {

	int retb;
	if (!gpio_is_ready_dt(&button_0)) {
		printk("Error: button 0 is not ready\n");
		return -1;
	}

	retb = gpio_pin_configure_dt(&button_0, GPIO_INPUT);
	gpio_pin_configure_dt(&button_1, GPIO_INPUT);
	gpio_pin_configure_dt(&button_2, GPIO_INPUT);
	gpio_pin_configure_dt(&button_3, GPIO_INPUT);
	gpio_pin_configure_dt(&button_4, GPIO_INPUT);
	if (retb != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	retb = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&button_1, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&button_2, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&button_3, GPIO_INT_EDGE_TO_ACTIVE);
	gpio_pin_interrupt_configure_dt(&button_4, GPIO_INT_EDGE_TO_ACTIVE);
	if (retb != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");
	gpio_init_callback(&button_1_data, button_1_handler, BIT(button_1.pin));
	gpio_add_callback(button_1.port, &button_1_data);

	gpio_init_callback(&button_2_data, button_2_handler, BIT(button_2.pin));
	gpio_add_callback(button_2.port, &button_2_data);

	gpio_init_callback(&button_3_data, button_3_handler, BIT(button_3.pin));
	gpio_add_callback(button_3.port, &button_3_data);

	gpio_init_callback(&button_4_data, button_4_handler, BIT(button_4.pin));
	gpio_add_callback(button_4.port, &button_4_data);
	
	return 0;
}


// Main program
int main(void)
{
	init_button();
	init_led();
	int ret = init_uart();
	if (ret != 0) {
		printk("UART initialization failed!\n");
		return ret;
	}
	return 0;
}

// Tasks to handle leds
void red_led_task(void *, void *, void*) {
	while(true){
		k_mutex_lock(&red_mutex, K_FOREVER);
        k_condvar_wait(&red_cv, &red_mutex, K_FOREVER);
        k_mutex_unlock(&red_mutex);	

		printk("Red led thread started\n");
		gpio_pin_set_dt(&red,1);
		printk("Red on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		printk("Red off\n");
		k_sleep(K_SECONDS(1));	
		//release signaali dispatcherille
		k_sem_give(&release_sem);
	}	
}
void yellow_led_task(void *, void *, void*) {
	while(true){
		k_mutex_lock(&yellow_mutex, K_FOREVER);
		k_condvar_wait(&yellow_cv, &yellow_mutex, K_FOREVER);
		k_mutex_unlock(&yellow_mutex);	

		printk("Yellow led thread started\n");
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		printk("Yellow on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		printk("Yellow off\n");
		k_sleep(K_SECONDS(1));
		
		k_sem_give(&release_sem);
	}
}
// Yellow blinking LED task
void yellow_blink_task(void *, void *, void*) {
	while(true){
		if(led_yellow_state == 1){
		printk("Yellow blink thread started\n");
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		printk("Yellow on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		printk("Yellow off\n");
		k_sleep(K_SECONDS(1));
	}
}
}
void green_led_task(void *, void *, void*) {
	while(true){
		k_mutex_lock(&green_mutex, K_FOREVER);
        k_condvar_wait(&green_cv, &green_mutex, K_FOREVER);
        k_mutex_unlock(&green_mutex);	

		printk("Green led thread started\n");
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&green,0);
		printk("Green off\n");
		k_sleep(K_SECONDS(1));

		k_sem_give(&release_sem);
	}	
}
void blue_led_task(void *, void *, void*) {
	while(true){
	if(led_state == 3){
		printk("Blue led thread started\n");
		gpio_pin_set_dt(&blue,1);
		printk("Blue on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&blue,0);
		printk("Blue off\n");
		k_sleep(K_SECONDS(1));
	}
	else
	k_msleep(100);
	}
}

static void uart_task(void *unused1, void *unused2, void *unused3)
{
    char rc = 0;
    char uart_msg[20];
    memset(uart_msg, 0, sizeof(uart_msg));
    int uart_msg_cnt = 0;

    while (true) {
        if (uart_poll_in(uart_dev, &rc) == 0) {
            if (rc != '\r') {
                if (uart_msg_cnt < sizeof(uart_msg) - 1) {
                    uart_msg[uart_msg_cnt++] = rc;
                }
            } else {
                // Go through the whole message and send the letters to fifo
                for (int i = 0; i < uart_msg_cnt; i++) {
                    char merkki = uart_msg[i];
                    if (merkki == 'R' || merkki == 'G' || merkki == 'Y') {
                        struct data_t *buf = k_malloc(sizeof(struct data_t));
                        if (!buf) {
                            printk("Malloc failed!\n");
                            break;  // Memory allocation failed
                        }
                        buf->color = merkki;
                        k_fifo_put(&dispatcher_fifo, buf);
                    }
                }
                uart_msg_cnt = 0;
                memset(uart_msg, 0, sizeof(uart_msg));
            }
        }
        k_msleep(10);
    }
}

// Dispatcher task
static void dispatcher_task(void *unused1, void *unused2, void *unused3)
{
	while (true) {
		// Receive dispatcher data from uart_task fifo
		struct data_t *rec_item = k_fifo_get(&dispatcher_fifo, K_FOREVER);
		char sequence[20];
		char color = rec_item->color;
		memcpy(sequence,rec_item->msg,20);
		k_free(rec_item);

		printk("Dispatcher: %s\n", color);
        // Parse color from the fifo data
        //char color = sequence[0];

        // Send the parsed color information to tasks using fifo
		switch (color) {
            case 'R':
                k_mutex_lock(&red_mutex, K_FOREVER);
                k_condvar_signal(&red_cv);
                k_mutex_unlock(&red_mutex);
                break;
            case 'G':
                k_mutex_lock(&green_mutex, K_FOREVER);
                k_condvar_signal(&green_cv);
                k_mutex_unlock(&green_mutex);
                break;
            case 'Y':
                k_mutex_lock(&yellow_mutex, K_FOREVER);
                k_condvar_signal(&yellow_cv);
                k_mutex_unlock(&yellow_mutex);
                break;
            default:
                printk("Invalid color: %c\n", color);
                continue;
        }
		k_sem_take(&release_sem, K_FOREVER);
        printk("Dispatcher: LED task completed\n");
	}
}
K_THREAD_DEFINE(dis_thread,STACKSIZE,dispatcher_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(uart_thread,STACKSIZE,uart_task,NULL,NULL,NULL,PRIORITY,0,0);