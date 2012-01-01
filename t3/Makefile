#***************************************************************************
#             Makefile
# 
#  	Wed Aug 31 10:33:22 2011
#   Copyright  2011  Karolina, Patricia, Pedro
#   Email: {karol.milano, lvlayumi, ptersnow}@gmail.com
#
#***************************************************************************

.SUFFIXES: .o .c
.PHONY: clean all

CC = gcc
CFLAGS = -g -lpthread -pedantic -Wall -lrt

#***************************************************************************

all: clean httpd

clean:
	rm -f httpd

httpd:
	$(CC) $(CFLAGS) *.c -o $@
