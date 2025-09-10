/*
Tehtävien pisteet
Tehtävä 1, 2p.
LEDit kiertävät eri värit punainen -> keltainen -> vihreä -> punainen jne. Toteutus tehty taskien kautta käyttäen tilakonetta.
Napille tehty keskeytystoiminto, jolla led tila otetaan talteen, ja uudelleen painamalla tilojen kiertoa jatketaan.
*/

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

// configure buttons
#define BUTTON_0 DT_ALIAS(sw0)

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(BUTTON_0, gpios, {0});
static struct gpio_callback button_0_data;

// led thread initialization
#define STACKSIZE 500
#define PRIORITY 5


void red_led_task(void *, void *, void*);
void green_led_task(void *, void *, void*);
void blue_led_task(void *, void *, void*);
void yellow_led_task(void *, void *, void*);

K_THREAD_DEFINE(red_thread,STACKSIZE,red_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(green_thread,STACKSIZE,green_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(blue_thread,STACKSIZE,blue_led_task,NULL,NULL,NULL,PRIORITY,0,0);
K_THREAD_DEFINE(yellow_thread,STACKSIZE,yellow_led_task,NULL,NULL,NULL,PRIORITY,0,0);

int led_state;
int prev_led_state;

// Button interrupt handler
void button_0_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	printk("Pause button pressed\n");
	if(led_state != 5){
		prev_led_state = led_state;
		led_state = 5;
		k_thread_suspend(red_thread);
		k_thread_suspend(green_thread);
		k_thread_suspend(yellow_thread);
	} else{
		led_state = prev_led_state;
		k_thread_resume(red_thread);
		k_thread_resume(green_thread);
		k_thread_resume(yellow_thread);
	}
}

// Main program
int main(void)
{
	init_button();
	init_led();

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
	if (retb != 0) {
		printk("Error: failed to configure pin\n");
		return -1;
	}

	retb = gpio_pin_interrupt_configure_dt(&button_0, GPIO_INT_EDGE_TO_ACTIVE);
	if (retb != 0) {
		printk("Error: failed to configure interrupt on pin\n");
		return -1;
	}

	gpio_init_callback(&button_0_data, button_0_handler, BIT(button_0.pin));
	gpio_add_callback(button_0.port, &button_0_data);
	printk("Set up button 0 ok\n");
	
	return 0;
}

// Tasks to handle leds
void red_led_task(void *, void *, void*) {
	while(true){	
	if(led_state == 0){
		printk("Red led thread started\n");
		gpio_pin_set_dt(&red,1);
		printk("Red on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		printk("Red off\n");
		k_sleep(K_SECONDS(1));	
		led_state = 1;
		//if(led_state == 0){
		//	led_state = 1;
		//}
	}
	else
	k_msleep(100);
	}	
}
void yellow_led_task(void *, void *, void*) {
	while(true){
	if(led_state == 1){
		printk("Yellow led thread started\n");
		gpio_pin_set_dt(&red,1);
		gpio_pin_set_dt(&green,1);
		printk("Yellow on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&red,0);
		gpio_pin_set_dt(&green,0);
		printk("Yellow off\n");
		k_sleep(K_SECONDS(1));
		led_state = 2;
		/*if(led_state == 1){
			led_state = 2;
		}*/
	}
	else
	k_msleep(100);
	}
}
void green_led_task(void *, void *, void*) {
	while(true){
	if(led_state == 2){
		printk("Green led thread started\n");
		gpio_pin_set_dt(&green,1);
		printk("Green on\n");
		k_sleep(K_SECONDS(1));
		gpio_pin_set_dt(&green,0);
		printk("Green off\n");
		k_sleep(K_SECONDS(1));
		led_state = 0;
		/*if(led_state == 2){
			led_state = 0;
		}*/
	}
	else
	k_msleep(100);
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