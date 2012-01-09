# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DstmDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DstmDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldstm -ldmgr

all: $(TARGETS)

DstmDemo:
	$(CC) $(CFLAGS) -o DstmDemo DstmDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

