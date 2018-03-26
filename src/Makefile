MPICC=mpicc
VERSION=0.04

all:
	$(MPICC) -fopenmp -o game-of-life main.c helpers.c io_functions.c play.c init.c -lm


clean:
	rm -f *~ *.o *.bin game-of-life

release:
	tar -cvf game-of-life-$(VERSION).tar *.c *.h *m *sh Makefile