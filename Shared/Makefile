TARGET = libparachutedev.a
OBJS = log.o misc.o hexdump.o link.o fifolink.o linkfactory.o
CFLAGS = ${MYCFLAGS} -I.

all : $(TARGET)

.cpp.o:
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $<

misc.o: misc.cpp misc.h types.h log.h
log.o: log.cpp log.h
hexdump.o: hexdump.cpp types.h log.h
link.o: link.cpp link.h log.h types.h constants.h
fifolink.o: fifolink.cpp fifolink.h link.h log.h types.h constants.h
linkfactory.o: linkfactory.cpp linkfactory.h link.h fifolink.h log.h types.h constants.h

$(TARGET): $(OBJS)
	-rm -f $(TARGET)
	ar q $(TARGET) $(OBJS)

clean:
	rm -f $(OBJS) $(TARGET)

