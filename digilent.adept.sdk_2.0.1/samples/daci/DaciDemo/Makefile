# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DaciDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DaciDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldaci -ldmgr

all: $(TARGETS)

DaciDemo:
	$(CC) $(CFLAGS) -o DaciDemo DaciDemo.cpp
	

.PHONY: vclean

vclean:
	rm $(TARGETS)

