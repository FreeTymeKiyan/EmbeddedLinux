# Makefile of etch_a_sketch
etch_a_sketch: etch_a_sketch.o gpio-utils.o
	gcc -o etch_a_sketch etch_a_sketch.o gpio-utils.o

etch_a_sketch.o: etch_a_sketch.c gpio-utils.o
	gcc -c etch_a_sketch.c

gpio-utils.o: gpio-utils.c gpio-utils.h
	gcc -c gpio-utils.c

