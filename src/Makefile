IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR)

ODIR=obj
LDIR =../lib

_DEPS = idealEngine.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = idealEngine.o master.o worker.o semUtils.o userFuncts.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

idealEngine: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) -lpthread -lrt

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~; \
	rm -f idealEngine


.PHONY: run
run:
	./idealEngine

.PHONY: debug
debug:
	gdb ./idealEngine

.PHONY: cr
cr:
	$(MAKE) clean	
	$(MAKE) 	
	$(MAKE) run

c:
	$(MAKE) clean	
	$(MAKE) 
