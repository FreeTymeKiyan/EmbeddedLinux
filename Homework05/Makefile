TOOLS_CFLAGS	:= -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-qual \
		   -Wcast-align -Wwrite-strings -Wnested-externs -Winline \
		   -W -Wundef -Wmissing-prototypes
CFLAGS  := -O3 -g
#
# Programs
#
all:	gpiotest gpioToggle controlLED 

gpiotest:  gpiotest.c beaglebone_gpio.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

gpioToggle:  gpioToggle.c beaglebone_gpio.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

controlLED:  controlLED.o beaglebone_gpio.h gpio-utils.o
	gcc -o controlLED controlLED.o beaglebone_gpio.h gpio-utils.o

controlLED.o: controlLED.c gpio-utils.o
	gcc -c controlLED.c

gpio-utils.o: gpio-utils.c gpio-utils.h
	gcc -c gpio-utils.c


#
# Objects
#

%.o: %.c
	$(CC) $(CFLAGS) $(TOOLS_CFLAGS) -c $< -o $@

clean:
	rm gpiotest gpioToggle

print:
	@echo LDFLAGS = $(LDFLAGS)
