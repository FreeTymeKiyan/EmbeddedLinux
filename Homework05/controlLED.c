//write a C program that reads from at least two switches and controls two LEDs (the built-in LEDs are fine). 
//The gpio pins used for the switches need to be from two different gpio ports. 
//This means you will have to use two separate mmap() calls.

// read switches
// gpio out is LEDS, control LEDs
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "beaglebone_gpio.h"

#define GPIO_31 (1<<31)
#define GPIO_50 (1<<18)

int keepgoing = 1;

void signal_handler(int sig);

void signal_handler(int sig) {
	printf( "\nCtrl-C pressed, cleaning up and exiting...\n" );
	keepgoing = 0;
}

int main(int argc, int argv[]) {

	volatile void *gpio_addr, *gpio_addr1;
	volatile unsigned int *gpio_datain, *gpio_datain1;
	volatile unsigned int *gpio_setdataout_addr;
	volatile unsigned int *gpio_cleardataout_addr;

	signal(SIGINT, signal_handler);

	system("./setup.sh");

	int fd = open("/dev/mem", O_RDWR);
	gpio_addr = mmap(0, GPIO0_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO0_START_ADDR);
	gpio_addr1 = mmap(1, GPIO1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO1_START_ADDR);
	gpio_datain            = gpio_addr  + GPIO_DATAIN;
	gpio_datain1	       = gpio_addr1 + GPIO_DATAIN;
	gpio_setdataout_addr   = gpio_addr1 + GPIO_SETDATAOUT;
	gpio_cleardataout_addr = gpio_addr1 + GPIO_CLEARDATAOUT;

	if(gpio_addr == MAP_FAILED) {
	        printf("Unable to map GPIO\n");
	        exit(1);
	}

	while(keepgoing) {
		if(!(*gpio_datain & GPIO_31)){
			*gpio_setdataout_addr = USR3;
		} else {
			*gpio_cleardataout_addr = USR3;
		}
		usleep(1000);
		
		if(*gpio_datain1 & GPIO_50){
			*gpio_setdataout_addr = USR3;
		} else {
			*gpio_cleardataout_addr = USR3;
		}
		usleep(1000);
	}

	close(fd);
	return 0;
}
