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

PROXY = cache.c config.c httpd.c list.c post.c request.c server.c signals.c
PROXY_OBJS = $(PROXY:.c=.o)



#***************************************************************************

all: clean httpd

clean:
	rm -f httpd $(PROXY_OBJS)

httpd: $(PROXY_OBJS)
	$(CC) $(CFLAGS) -o $@ $^
