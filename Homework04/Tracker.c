/**
 * Author: FreeTymeKiyan
 * Date: 2013-09-30
 *
 * Step motor and IR Tracker
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include "gpio.h"

void init_gpios();
void clockwise_rotate(int *pos);
void counter_clockwise_rotate(int *pos);
int analog_in(char *ain);
void signal_handler(int sig);
int mode_gpio_out(char *pinMux);

#define PIN_MUX_PATH "/sys/kernel/debug/omap_mux/"
#define MAX_BUF 64

int keepgoing = 1;
unsigned int controller[4] = {30, 31, 48, 51};

int main(int argc, char *argv[]) {

	char PT1[] = "AIN4", PT2[] = "AIN6";
	int PT1_val[20], PT2_val[20], PT_sum[20];
	int min, min_pos, pos, PT1_now, PT2_now, i;

	signal(SIGINT, signal_handler);

	init_gpios();

	pos = 0;
	//clockwise_rotate(&pos);
	for(i = 0; i < 20; i++) {
		PT1_val[i] = analog_in(PT1);
		PT2_val[i] = analog_in(PT2);
		printf("PT1: %4d\tPT2: %4d\n", PT1_val[i], PT2_val[i]);
		clockwise_rotate(&pos);
		usleep(1000 * 100);
	}

	for(i = 0; i < 20; i++) {
		PT_sum[i] = PT1_val[i] + PT2_val[i];
	}

	min = PT_sum[0];
	for(i = 0; i < 20; i++) {
		if(PT_sum[i] < min) {
			min = PT_sum[i];
			min_pos = i;
		}
	}

	printf("minimum: %d\tminimum_pos: %d\n", min, min_pos);

	for(i = 19; i >= 0; i--) {
		counter_clockwise_rotate(&pos);
		usleep(1000 * 100);
		if((i - min_pos) == 0) break;
	}

	while(keepgoing) {
		PT1_now = analog_in(PT1);
		usleep(1000 * 100);
		PT2_now = analog_in(PT2);

		if(PT1_now - PT2_now < 150 && PT1_now - PT2_now > -150) {
			usleep(1000 * 100);
			//continue;
		} else if(PT1_now > PT2_now) {
			clockwise_rotate(&pos);
		} else if(PT1_now < PT2_now) {
			counter_clockwise_rotate(&pos);
		}
		printf("PT1: %4d\tPT2: %4d\n", PT1_now, PT2_now);
		usleep(1000 * 300);
	}
	return 0;	
}

void signal_handler(int sig) {
	int i;
	printf("Ctrl-C pressed, exiting...\n");
	keepgoing = 0;

	for(i = 0; i < 4; i++) {
		gpio_unexport(controller[i]);

	}
}

void init_gpios() {
	char gpio_30[] = "gpmc_wait0";
	char gpio_31[] = "gpmc_wpn";
	char gpio_48[] = "gpmc_a0";
	char gpio_51[] = "spi0_cs0";

	mode_gpio_out(gpio_30);
	mode_gpio_out(gpio_31);
	mode_gpio_out(gpio_48);
	mode_gpio_out(gpio_51);
	
	int i;
	for(i = 0; i < 4; i++) {
		gpio_export(controller[i]);
		gpio_set_direction(controller[i], 1);
	}
}

int mode_gpio_out(char *pinMux) {
	int fd, len;
	char buf[MAX_BUF];
	len = snprintf(buf, sizeof(buf), PIN_MUX_PATH "%s", pinMux);
				 
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("mode/gpio");
		return fd;
	}
			 
	write(fd, "7", 2);	
	close(fd);
	return 0;
}

void clockwise_rotate(int *pos) {
	switch(*pos) {
		case 0:
			gpio_set_value(controller[0], 0);
			gpio_set_value(controller[1], 1);
			gpio_set_value(controller[2], 1);
			gpio_set_value(controller[3], 0);
			break;
		case 1:
			gpio_set_value(controller[0], 1);
			gpio_set_value(controller[1], 1);	
			gpio_set_value(controller[2], 0);
			gpio_set_value(controller[3], 0);
			break;
		case 2:
			gpio_set_value(controller[0], 1);
			gpio_set_value(controller[1], 0);
			gpio_set_value(controller[2], 0);
			gpio_set_value(controller[3], 1);
			break;
		case 3:
			gpio_set_value(controller[0], 0);
			gpio_set_value(controller[1], 0);
			gpio_set_value(controller[2], 1);
			gpio_set_value(controller[3], 1);
			break;
	}
	usleep(1000 * 50);
	*pos = (*pos+1)%4;
}

void counter_clockwise_rotate(int *pos) {
	switch(*pos) {
		case 0:
			gpio_set_value(controller[0], 1);
			gpio_set_value(controller[1], 0);
			gpio_set_value(controller[2], 0);
			gpio_set_value(controller[3], 1);
			break;
		case 1:
			gpio_set_value(controller[0], 0);
			gpio_set_value(controller[1], 0);	
			gpio_set_value(controller[2], 1);
			gpio_set_value(controller[3], 1);
			break;
		case 2:
			gpio_set_value(controller[0], 0);
			gpio_set_value(controller[1], 1);
			gpio_set_value(controller[2], 1);
			gpio_set_value(controller[3], 0);
			break;
		case 3:
			gpio_set_value(controller[0], 1);
			gpio_set_value(controller[1], 1);
			gpio_set_value(controller[2], 0);
			gpio_set_value(controller[3], 0);
			break;
	}
	usleep(1000 * 50);
	*pos = (*pos-1+4)%4;
}

int analog_in(char *ain) {
	FILE *fp;
	char ain_path[MAX_BUF];
	char ain_val[MAX_BUF];

	snprintf(ain_path, sizeof ain_path, "/sys/devices/ocp.2/helper.14/%s", ain);

	if((fp = fopen(ain_path, "r")) == NULL) {
		printf("Can't open this pin, %s\n", ain);
		return 1;
	}

	fgets(ain_val, MAX_BUF, fp);
	fclose(fp);
	return atoi(ain_val);
}
