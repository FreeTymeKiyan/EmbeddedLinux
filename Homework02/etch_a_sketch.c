#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	
#include "gpio-utils.h"

#define SIZE 8 
#define MAX_BUF 64

void after_move();
void clear();

char disp_matrix[SIZE][SIZE];
int x;
int y;
int initialized = 0;

// show welcome info
void welcome() {
	system("clear");
	printf("=============================================================\n");
	printf("||This is an Etch-A-Sketch program written by FreeTymeKiyan||\n");
	printf("=============================================================\n");
}

// do some initialization
void init() {
	//printf("init begin\n");
	// init disp_matrix[x][y]
	int i, j;
	for(i = 0; i < SIZE; i++) {
		for(j = 0; j < SIZE; j++ ){
			disp_matrix[i][j] = '0';
		}
		// printf("i:%d, j:%d", i, j);
		j = 0;
	}
	x = 0;
	y = 0;
	//printf("init end\n");
}

// draw the screen according to the array
void draw() {
	//printf("draw begin\n");
	system("clear");
	welcome();
	if(initialized == 0) {
		clear();
	}
	else {
		int i, j;
		for(i = 0; i < SIZE; i++) {
			for(j = 0; j < SIZE; j++)
				printf("%c", disp_matrix[i][j]);
			j = 0;
			printf("\n");
		}
	}
	//printf("draw end\n");
}

// move up
void move_up() {
	y--;
	if(y >= 0) {
		after_move();
		draw();
	}
	else {
		y = 0;
		printf("\nhit the upper edge\n");
	}
} 

// move down
void move_down() {
	y++;
	if(y <= 7) {
		after_move();
		draw();
	}
	else {
		y = 7;
		printf("\nhit the lower edge\n");
	}
}

// move left
void move_left() {
	x--;
	if(x >= 0) {
		after_move();
		draw();
	}
	else {
		x = 0;
		printf("\nhit the left edge!\n");
	}
}

// move right
void move_right() {
	x++;
	if(x <= 7) {
		after_move();
		draw();
	}
	else {
		x = 7;
		printf("\nhit the right edge!\n");
	}
}

// clear screen
void clear() {
	//printf("clear begin\n");
	int i, j;
	for(i = 0; i < SIZE; i++) {
		for(j = 0; j < SIZE; j++ ){
			disp_matrix[i][j] = '0';
		}
		j = 0;
	}
	//printf("clear end\n");
}

// set the disp_matrix after move
void after_move() {
	disp_matrix[y][x] = 'X'; 
}

// set gpio
int set_gpio(int gpio_no) {
	gpio_export(gpio_no);
	gpio_set_dir(gpio_no, "in");
	gpio_set_edge(gpio_no, "falling");
	return gpio_fd_open(gpio_no, O_RDONLY);
}

// starts here
int main(int argc, char** argv) {
	// usage notice
	if (argc != 5){
		printf("Usage: %s <left> <down> <up> <right>\n\n", argv[0]);
		exit(-1);
	}
		
	init();

	struct pollfd fdset[4];
	unsigned int gpio;
	int rc, timeout;
	int gpio_fd[4];
	char buf[MAX_BUF];
	int len;
	int nfds = 5;

	int i;
	for(i = 0; i < 4; i++) {
		//printf("%d", atoi(argv[i+1]));
		gpio_fd[i] = set_gpio(atoi(argv[i + 1]));
	}

	timeout = 3 * 1000;

	while(1) {
		memset((void*)fdset, 0, sizeof(fdset));
		int i;
		for(i = 0; i < 4; i++) {
			fdset[i].fd = gpio_fd[i];
			fdset[i].events = POLLPRI;
		}
		rc = poll(fdset, nfds, timeout);  
		//printf("rc is : %d", rc);
		if (rc < 0) {
			printf("\npoll() failed!\n");
			return -1;
		}
   	
		if (rc == 0) {
			printf(".");
		}	
	
		if(fdset[0].revents & POLLPRI) {
			lseek(fdset[0].fd, 0, SEEK_SET);  
			read(fdset[0].fd, buf, MAX_BUF);
			//printf("left button pushed\n");
			move_left();
		}
		if(fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);  
			read(fdset[1].fd, buf, MAX_BUF);
			//printf("down button pushed\n");
			move_down();
		}
		if(fdset[2].revents & POLLPRI) {
			lseek(fdset[2].fd, 0, SEEK_SET);  
			read(fdset[2].fd, buf, MAX_BUF);
			//printf("up button pushed\n");
			move_up();
		}
		if(fdset[3].revents & POLLPRI) {
			lseek(fdset[3].fd, 0, SEEK_SET);  
			read(fdset[3].fd, buf, MAX_BUF);
			//printf("right button pushed\n");
			move_right();
		}
		//if (initialized == 0) clear();
		initialized = 1;
		fflush(stdout);
	}
	for(i = 0; i < 4; i++)
		gpio_fd_close(gpio_fd[i]);
	return 0;
}
