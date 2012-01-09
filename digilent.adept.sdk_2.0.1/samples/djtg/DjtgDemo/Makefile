# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DjtgDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DjtgDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldjtg -ldmgr

all: $(TARGETS)

DjtgDemo:
	$(CC) $(CFLAGS) -o DjtgDemo DjtgDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

