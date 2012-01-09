# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DemcStepDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DemcStepDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldemc -ldmgr

all: $(TARGETS)

DemcStepDemo:
	$(CC) $(CFLAGS) -o DemcStepDemo DemcStepDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

