DIRS=Shared NodeServer Emulator

export PARACHUTEPREFIX=/opt/parachute
export BINDIR = $(PARACHUTEPREFIX)/bin
export LIBDIR = $(PARACHUTEPREFIX)/lib
export INCDIR = $(PARACHUTEPREFIX)/include


export VERSION = $(shell cat VERSION.txt)
export CC = g++
export MYCFLAGS = -O2 -g -DDEBUG -DVERSION=${VERSION}

all:
	@echo "Making all in subdirectories..."
	@for i in $(DIRS); do (echo "Make all in $$i"; cd $$i; make all) || exit 1; done

install:
	@sudo mkdir -p $(PARACHUTEPREFIX)
	@sudo mkdir -p $(PARACHUTEPREFIX)/lib
	@sudo mkdir -p $(PARACHUTEPREFIX)/bin
	@sudo mkdir -p $(PARACHUTEPREFIX)/include
	@sudo mkdir -p $(PARACHUTEPREFIX)/man
	@sudo mkdir -p $(PARACHUTEPREFIX)/info
	@echo "Making install in subdirectories..."
	@for i in $(DIRS); do (echo "Make install in $$i"; cd $$i; make install) || exit 1; done

clean:
	@echo "Making clean in subdirectories..."
	@for i in $(DIRS); do (echo "Make clean in $$i"; cd $$i; make clean) || exit 1; done




