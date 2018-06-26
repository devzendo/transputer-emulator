DIRS=Shared NodeServer Emulator
PARACHUTEPREFIX=/opt/parachute

all: setup toolchain parachute

setup:
	@sudo mkdir -p $(PARACHUTEPREFIX)
	@sudo mkdir -p $(PARACHUTEPREFIX)/lib
	@sudo mkdir -p $(PARACHUTEPREFIX)/bin
	@sudo mkdir -p $(PARACHUTEPREFIX)/include
	@sudo mkdir -p $(PARACHUTEPREFIX)/man
	@sudo mkdir -p $(PARACHUTEPREFIX)/info

parachute:
	@echo "Making all in subdirectories..."
	@for i in $(DIRS); do (cd $$i; make all) || exit 1; done

toolchain:
	@echo "Making all in ToolChain..."
	@(cd ToolChain; make all) || exit 1

clean:
	@echo "Making clean in subdirectories..."
	@for i in $(DIRS); do (cd $$i; make clean) || exit 1; done




