all: common single thumbnail

common single thumbnail:
	$(MAKE) -C $@

clean:
	$(MAKE) -C single clean
	$(MAKE) -C thumbnail clean
	$(MAKE) -C common clean

.PHONY: all common single thumbnail clean
