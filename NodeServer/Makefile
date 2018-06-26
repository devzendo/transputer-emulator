DIRS=server client

all:
	@echo "Making all in subdirectories..."
	@for i in $(DIRS); do (cd $$i; make all) || exit 1; done

clean:
	@echo "Making clean in subdirectories..."
	@for i in $(DIRS); do (cd $$i; make clean) || exit 1; done




