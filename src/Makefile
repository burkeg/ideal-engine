IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

_DEPS = idealEngine.h master.h worker.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = idealEngine.o # hellofunc.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

idealEngine: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) -lpthread -lrt

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 

run:
	./idealEngine
