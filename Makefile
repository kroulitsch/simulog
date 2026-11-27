all: run

api:
	$(MAKE) -C backend

run: api
	$(MAKE) -C frontend run

clean:
	$(MAKE) -C backend clean
	$(MAKE) -C frontend clean

%:
	@:

.PHONY: all qt api clean