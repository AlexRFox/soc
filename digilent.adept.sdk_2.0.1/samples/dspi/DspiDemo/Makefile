# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DspiDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DspiDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldspi -ldmgr

all: $(TARGETS)

DspiDemo:
	$(CC) $(CFLAGS) -o DspiDemo DspiDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

