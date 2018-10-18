EXES = temulate
OBJS = main.o memory.o cpu.o disasm.o
SHARED = ../Shared
CFLAGS = ${MYCFLAGS} -I$(SHARED)
LDFLAGS = -L$(SHARED)
LIBS = -lparachutedev

all : $(EXES)

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

main.o: main.cpp $(SHARED)/types.h $(SHARED)/constants.h flags.h $(SHARED)/memloc.h memory.h cpu.h $(SHARED)/log.h $(SHARED)/linkfactory.h $(SHARED)/link.h
memory.o: memory.cpp $(SHARED)/types.h $(SHARED)/constants.h memory.h $(SHARED)/memloc.h flags.h $(SHARED)/log.h
cpu.o: cpu.cpp $(SHARED)/types.h cpu.h $(SHARED)/link.h $(SHARED)/linkfactory.h memory.h $(SHARED)/memloc.h $(SHARED)/log.h $(SHARED)/constants.h flags.h opcodes.h disasm.h
disasm.o: disasm.cpp $(SHARED)/types.h $(SHARED)/constants.h $(SHARED)/log.h flags.h opcodes.h

temulate : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

install: temulate
	sudo install -o root -g staff -m 755 temulate $(BINDIR)

clean:
	rm -f $(OBJS) $(EXES)

test:
	@echo No tests here
