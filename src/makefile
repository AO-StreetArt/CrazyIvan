# Makefile is defined to call make within the app, api, and model directories

.PHONY: mkapp mkapi mkmodel mkproc

all: mkmodel mkapi mkproc mkapp

mkapp:
	@$(MAKE) -C app

mkapi:
	@$(MAKE) -C api

mkmodel:
	@$(MAKE) -C model

mkproc:
	@$(MAKE) -C proc
