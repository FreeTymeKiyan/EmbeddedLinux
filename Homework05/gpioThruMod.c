// From : http://stackoverflow.com/questions/13124271/driving-beaglebone-gpio-through-dev-mem
//
// Read one gpio pin and write it out to another using mmap.
// Be sure to set -O3 when compiling.
// Modified by Mark A. Yoder  26-Sept-2013

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h>    // Defines signal-handling functions (i.e. trap Ctrl-C)
#include "beaglebone_gpio.h"

// modified by FreeTymeKiyan 2013-10-07 begin
#define GPIO_50		(1<<18)
// modified by FreeTymeKiyan 2013-10-07 end

/****************************************************************
 * Global variables
 ****************************************************************/
int keepgoing = 1;    // Set to 0 when ctrl-c is pressed

/****************************************************************
 * signal_handler
 ****************************************************************/
void signal_handler(int sig);
// Callback called when SIGINT is sent to the process (Ctrl-C)
void signal_handler(int sig)
{
    printf( "\nCtrl-C pressed, cleaning up and exiting...\n" );
	keepgoing = 0;
}

int main(int argc, char *argv[]) {
    volatile void *gpio_addr, *gpio_addr1;
    volatile unsigned int *gpio_oe_addr;
    volatile unsigned int *gpio_datain;
    volatile unsigned int *gpio_setdataout_addr;
    volatile unsigned int *gpio_cleardataout_addr;
    unsigned int reg;

    // Set the signal callback for Ctrl-C
    signal(SIGINT, signal_handler);

    int fd = open("/dev/mem", O_RDWR);

    printf("Mapping %X - %X (size: %X)\n", GPIO0_START_ADDR, GPIO0_END_ADDR, 
                                           GPIO0_SIZE);

    // modified by FreeTymeKiyan 2013-10-07 begin 
    gpio_addr1 = mmap(1, GPIO1_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 		GPIO1_START_ADDR);
    // modified by FreeTymeKiyan 2013-10-07 end
		

    gpio_oe_addr           = gpio_addr1 + GPIO_OE;
    gpio_datain            = gpio_addr1 + GPIO_DATAIN;
    gpio_setdataout_addr   = gpio_addr1 + GPIO_SETDATAOUT;
    gpio_cleardataout_addr = gpio_addr1 + GPIO_CLEARDATAOUT;

    if(gpio_addr == MAP_FAILED) {
        printf("Unable to map GPIO\n");
        exit(1);
    }
    printf("GPIO mapped to %p\n", gpio_addr);
    printf("GPIO OE mapped to %p\n", gpio_oe_addr);
    printf("GPIO SETDATAOUTADDR mapped to %p\n", gpio_setdataout_addr);
    printf("GPIO CLEARDATAOUT mapped to %p\n", gpio_cleardataout_addr);

    while(keepgoing) {
	// modified by FreeTymeKiyan 2013-10-07 begin
    	if(*gpio_datain & GPIO_50) {
            *gpio_setdataout_addr= USR3;
    	} else {
            *gpio_cleardataout_addr = USR3;
    	}
	// modified by FreeTymeKiyan 2013-10-07 end
    }

    munmap((void *)gpio_addr1, GPIO1_SIZE);
    close(fd);
    return 0;
}
