# Makefile is defined to call make within the app, api, and model directories

.PHONY: mkapi mkmodel mkproc

all: mkmodel mkapi mkproc

mkapi:
	@$(MAKE) -C api

mkmodel:
	@$(MAKE) -C model

mkproc:
	@$(MAKE) -C proc
