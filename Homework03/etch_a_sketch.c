#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>	
#include "gpio-utils.h"
#include "i2c-dev.h"
#include "i2cbusses.h"

#define SIZE 8 
#define MAX_BUF 64
#define BICOLOR

void after_move();
void clear();

char disp_matrix[SIZE][SIZE];
int x;
int y;
int initialized = 0;

// matrix
__u16 screen[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static int check_funcs(int file) {
        unsigned long funcs;

        /* check adapter functionality */
        if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
        	fprintf(stderr, "Error: Could not get the adapter "
                        "functionality matrix: %s\n", strerror(errno));
	   	return -1;
	}

	if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
	        fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
	        return -1;
	}
	return 0;
}

// Writes block of data to the display
static int write_block(int file, __u16 *data) {
        int res;
#ifdef BICOLOR
        res = i2c_smbus_write_i2c_block_data(file, 0x00, 16,
                (__u8 *)data);
        return res;
#else
/*
 *  * For some reason the single color display is rotated one column,
*   * so pre-unrotate the data.
*    */
        int i;
        __u16 block[I2C_SMBUS_BLOCK_MAX];
	printf("rotating\n");
	for(i=0; i<8; i++) {
		block[i] = (data[i]&0xfe) >> 1 |
		(data[i]&0x01) << 7;
	}
	res = i2c_smbus_write_i2c_block_data(file, 0x00, 16,
		(__u8 *)block);
	return res;
#endif
}

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
void draw(int file) {
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
	// show on the matrix
	write_block(file, screen);
	// show on the matrix
	//printf("draw end\n");
}

// move up
void move_up(int file) {
	y--;
	if(y >= 0) {
		after_move();
		draw(file);
	}
	else {
		y = 0;
		printf("\nhit the upper edge\n");
	}
} 

// move down
void move_down(int file) {
	y++;
	if(y <= 7) {
		after_move();
		draw(file);
	}
	else {
		y = 7;
		printf("\nhit the lower edge\n");
	}
}

// move left
void move_left(int file) {
	x--;
	if(x >= 0) {
		after_move();
		draw(file);
	}
	else {
		x = 0;
		printf("\nhit the left edge!\n");
	}
}

// move right
void move_right(int file) {
	x++;
	if(x <= 7) {
		after_move();
		draw(file);
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
		screen[i] = 0x00;
	}
	//printf("clear end\n");
//	screen = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
}

// set the disp_matrix after move
void after_move() {
	disp_matrix[y][x] = 'X'; 
	// change the screen matrix
	screen[x] = screen[x] | (0x80 >> y);
	// check 
	int i;
	for(i = 0; i < 8; i++) {
		printf("%X ", screen[i]);
	}	
	printf("\n");
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

	// added by FreeTymeKiyan 2013-09-25 begin
	int force = 0;
	int i2cbus = lookup_i2c_bus("1");
	int address = parse_i2c_address("0x70");
	char filename[20];
	int file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);

	if (file < 0 || check_funcs(file) || set_slave_addr(file, address, force))
   		exit(1);

	// Check the return value on these if there is trouble
	i2c_smbus_write_byte(file, 0x21); // Start oscillator (p10)
	i2c_smbus_write_byte(file, 0x81); // Disp on, blink off (p11)
	i2c_smbus_write_byte(file, 0xe7); // Full brightness (page 15)
	// added by FreeTymeKiyan 2013-09-25 begin
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
			move_left(file);
		}
		if(fdset[1].revents & POLLPRI) {
			lseek(fdset[1].fd, 0, SEEK_SET);  
			read(fdset[1].fd, buf, MAX_BUF);
			//printf("down button pushed\n");
			move_down(file);
		}
		if(fdset[2].revents & POLLPRI) {
			lseek(fdset[2].fd, 0, SEEK_SET);  
			read(fdset[2].fd, buf, MAX_BUF);
			//printf("up button pushed\n");
			move_up(file);
		}
		if(fdset[3].revents & POLLPRI) {
			lseek(fdset[3].fd, 0, SEEK_SET);  
			read(fdset[3].fd, buf, MAX_BUF);
			//printf("right button pushed\n");
			move_right(file);
		}
		//if (initialized == 0) clear();
		initialized = 1;
		fflush(stdout);
	}
	for(i = 0; i < 4; i++)
		gpio_fd_close(gpio_fd[i]);
	return 0;
}
