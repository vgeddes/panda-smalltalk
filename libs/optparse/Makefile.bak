
CC=gcc
CFLAGS=-Wall -ansi -pedantic

all: fudd notpython doc
doc: optparse.dvi optparse.pdf
fudd: fudd.o optparse.o
notpython: notpython.o optparse.o
fudd.o: optparse.h
notpython.o: optparse.h
optparse.o: optparse.h

.SUFFIXES: .pdf
.tex.dvi:
	latex $<
	-grep Rerun $*.log && latex $<
.dvi.pdf:
	dvipdf $<
