# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DgioDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DgioDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldgio -ldmgr

all: $(TARGETS)

DgioDemo:
	$(CC) $(CFLAGS) -o DgioDemo DgioDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

