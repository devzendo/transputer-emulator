EXES = nodeserver
OBJS = nodeserver.o
EMPATH = ../../Emulator
SHARED = ../../Shared
SHOBJS = $(SHARED)/link.o $(SHARED)/fifolink.o $(SHARED)/linkfactory.o
CFLAGS = ${MYCFLAGS} -I.. -I$(SHARED)
LDFLAGS = -L$(SHARED)
LIBS = -lparachutedev

all : $(EXES)

install : $(EXES)
	sudo install -o root -g staff -m 755 nodeserver $(BINDIR)
	
.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

nodeserver.o: nodeserver.cpp $(SHARED)/log.h $(SHARED)/linkfactory.h $(SHARED)/link.h $(SHARED)/hexdump.h ../nsproto.h

nodeserver : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(SHOBJS) $^ $(LIBS)

clean:
	rm -f $(OBJS) $(EXES)

