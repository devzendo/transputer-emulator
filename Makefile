DIRS=Shared NodeServer Emulator
PARACHUTEPREFIX=/opt/parachute

all: setup parachute

setup:
	@sudo mkdir -p $(PARACHUTEPREFIX)
	@sudo mkdir -p $(PARACHUTEPREFIX)/lib
	@sudo mkdir -p $(PARACHUTEPREFIX)/bin
	@sudo mkdir -p $(PARACHUTEPREFIX)/include
	@sudo mkdir -p $(PARACHUTEPREFIX)/man
	@sudo mkdir -p $(PARACHUTEPREFIX)/info

parachute:
	@echo "Making all in subdirectories..."
	@for i in $(DIRS); do (echo "Make all in $$i"; cd $$i; make all) || exit 1; done

install:
	@echo "Making install in subdirectories..."
	@for i in $(DIRS); do (echo "Make install in $$i"; cd $$i; make install) || exit 1; done

clean:
	@echo "Making clean in subdirectories..."
	@for i in $(DIRS); do (echo "Make clean in $$i"; cd $$i; make clean) || exit 1; done




