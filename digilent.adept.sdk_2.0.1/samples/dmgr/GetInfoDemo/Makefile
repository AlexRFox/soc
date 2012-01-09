# File: makefile
# Author: Aaron Odell
# Company: Digilent Inc.
# Date: 8/16/2010
# Description: makefile for Adept SDK GetInfoDemo

CC = gcc
INC = /usr/local/include/digilent/adept
LIBDIR = /usr/local/lib/digilent/adept
TARGETS = GetInfoDemo
CFLAGS = -I $(INC) -L $(LIBDIR) -ldmgr

all: $(TARGETS)

GetInfoDemo:
	$(CC) $(CFLAGS) -o GetInfoDemo GetInfoDemo.cpp
	

.PHONY: vclean

vclean:
	rm -f $(TARGETS)

