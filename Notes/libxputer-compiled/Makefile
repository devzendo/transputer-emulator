SHELL	= /bin/sh

CC		= t800-gcc
CPPFLAGS	= -I.
CFLAGS		= -O2 -nostdlib

OBJS		= alt.o process.o sema.o misc.o
TARGET 		= libxputer.a

PREFIX		= /usr/local/t800
LIBDIR		= $(PREFIX)/lib
INCDIR		= $(PREFIX)/include

DISTFILES	= Makefile README README.koi8 xputer.h alt.c process.c sema.c misc.c 
DISTNAME 	= libxputer
DISTVERSION	= 0.05

all: $(TARGET)

libxputer.a: $(OBJS)
	-rm -f $(TARGET)
	ar q $(TARGET) $(OBJS)

clean:
	-rm -f *.[aois] *~

install: all
	install -o root -g root -m644 libxputer.a $(LIBDIR)
	install -o root -g root -m644 xputer.h $(INCDIR)


.c.s:
	$(CC) -S  $(CFLAGS) $(CPPFLAGS) -o $@ $<


dist:
	tar czf $(DISTNAME)-$(DISTVERSION).tar.gz $(DISTFILES)

