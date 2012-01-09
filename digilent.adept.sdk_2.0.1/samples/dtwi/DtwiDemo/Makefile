# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DtwiDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DtwiDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldtwi -ldmgr

all: $(TARGETS)

DtwiDemo:
	$(CC) $(CFLAGS) -o DtwiDemo DtwiDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

