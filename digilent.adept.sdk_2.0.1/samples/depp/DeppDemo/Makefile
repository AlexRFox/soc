# File: Makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DeppDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DeppDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldepp -ldmgr

all: $(TARGETS)

DeppDemo:
	$(CC) $(CFLAGS) -o DeppDemo DeppDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

