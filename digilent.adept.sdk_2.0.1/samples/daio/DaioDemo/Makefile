# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DaioDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DaioDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldaio -ldmgr

all: $(TARGETS)

DaioDemo:
	$(CC) $(CFLAGS) -o DaioDemo DaioDemo.cpp
	

.PHONY: vclean

vclean:
	rm $(TARGETS)

