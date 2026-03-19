# vim: set noet ts=8:
#
# This Makefile is not called from Opam but only used for
# convenience during development
#

.PHONY: all clean format
#
# K&R style indentation, could use in format target below
INDENT += -nbad -bap -nbc -br -brs -c33 -cd33 -ncdb -ce -ci4 -cli0
INDENT += -d0 -di1 -nfc1 -i4 -ip0 -l75 -lp -npcs
INDENT += -npsl -nsc -nsob

all:
	dune build @install --profile=release

build:
	dune build --profile=dev

install: all
	dune install --profile=release

clean:
	dune clean

format:
	dune build --auto-promote @fmt
	dune format-dune-file dune-project > $$$$ && mv $$$$ dune-project
	git ls-files '**/*.[ch]' | xargs -n1 indent -nut -i8
