#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

// Led pin configurations
static const struct gpio_dt_spec red = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec green = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
static const struct gpio_dt_spec blue = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);

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

// Main program
int main(void)
{
	init_led();

	return 0;
}

// Initialize leds
int  init_led() {
	led_state = 0;
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
		
		}
	else
	k_yield();
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
		
		}
	else
	k_yield();
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
	k_yield();
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
		
		}
	else
	k_yield();
	}
}