# Makefile for the main project
.PHONY: all clean client server

all: client server

client:
	@$(MAKE) -C client

server:
	@$(MAKE) -C server

runs:
	@$(MAKE) -C server run

runc:
	@$(MAKE) -C client run
