# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DpioDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DpioDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldpio -ldmgr

all: $(TARGETS)

DpioDemo:
	$(CC) $(CFLAGS) -o DpioDemo DpioDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

