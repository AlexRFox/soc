# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK DemcSrvDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = DemcSrvDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldemc -ldmgr

all: $(TARGETS)

DemcSrvDemo:
	$(CC) $(CFLAGS) -o DemcSrvDemo DemcSrvDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

