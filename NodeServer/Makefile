DIRS=server client

all:
	@echo "Making all in subdirectories..."
	@for i in $(DIRS); do (echo "NodeServer: Make all in $$i"; cd $$i; make all) || exit 1; done

install:
	@echo "Making install in subdirectories..."
	@for i in $(DIRS); do (echo "NodeServer: Make install in $$i"; cd $$i; make install) || exit 1; done

clean:
	@echo "Making clean in subdirectories..."
	@for i in $(DIRS); do (echo "NodeServer: Make clean in $$i"; cd $$i; make clean) || exit 1; done

test:
	@echo No tests here


