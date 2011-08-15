all: single

single:
	$(MAKE) -C single

clean:
	$(MAKE) -C single clean

.PHONY: all single clean
