all: common single

common single:
	$(MAKE) -C $@

clean:
	$(MAKE) -C single clean
	$(MAKE) -C common clean

.PHONY: all single common clean
