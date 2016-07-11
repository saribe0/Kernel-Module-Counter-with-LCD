# Dare_Dev
# EC 535, Spring 2016
# Lab 2, 2/24/16
# Makefile for kernal module user level interface ktimer

counterinfo : counterinfo.o
	arm-linux-gcc -o counterinfo counterinfo.o

counterinfo.o : counterinfo.c
	arm-linux-gcc -c counterinfo.c

clean :
	rm counterinfo counterinfo.o
