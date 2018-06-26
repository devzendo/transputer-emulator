EXES = t800emul
OBJS = main.o memory.o cpu.o disasm.o

VERSION = `cat ../VERSION.txt`
CC = g++
SHARED = ../Shared
CFLAGS = -O2 -g -DDEBUG -DVERSION=${VERSION} -I$(SHARED)
LDFLAGS = -L$(SHARED)
LIBS = -lparachutedev

PREFIX = /usr/local/t800
BINDIR = $(PREFIX)/bin

all : $(EXES)
	sudo install -o root -g root -m755 t800emul $(BINDIR)

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

main.o: main.cpp $(SHARED)/types.h $(SHARED)/constants.h flags.h $(SHARED)/memloc.h memory.h cpu.h $(SHARED)/log.h $(SHARED)/linkfactory.h $(SHARED)/link.h
memory.o: memory.cpp $(SHARED)/types.h $(SHARED)/constants.h memory.h $(SHARED)/memloc.h flags.h $(SHARED)/log.h
cpu.o: cpu.cpp $(SHARED)/types.h cpu.h $(SHARED)/link.h $(SHARED)/linkfactory.h memory.h $(SHARED)/memloc.h $(SHARED)/log.h $(SHARED)/constants.h flags.h opcodes.h disasm.h
disasm.o: disasm.cpp $(SHARED)/types.h $(SHARED)/constants.h $(SHARED)/log.h flags.h opcodes.h

t800emul : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

install: t800emul
	install -o root -g root -m755 t800emul $(BINDIR)

clean:
	rm -f $(OBJS) $(EXES)

