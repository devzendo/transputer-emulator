TARGET = libparachutedev.a
OBJS = log.o misc.o hexdump.o link.o fifolink.o linkfactory.o
TEST_TARGET = test_runner
TEST_OBJS = test_runner.o

SHARED = .
LDFLAGS = -L$(SHARED)
LIBS = -lparachutedev




CFLAGS = ${MYCFLAGS} -I.

all: $(TARGET)

test: $(TARGET) $(TEST_TARGET)

install:

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

$(TEST_TARGET): $(TEST_OBJS) $(TARGET)
	-rm -f $(TEST_TARGET)
	$(CC) -v $(LDFLAGS) $(TEST_OBJS) -o $(TEST_TARGET) $(LIBS)

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)

