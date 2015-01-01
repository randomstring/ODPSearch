SHELL=/bin/sh
#
# Compiler
#
CC=g++
CFLAGS=-g -DUNIX -Wall -D__CSTRING__
SRC_DIR=../src

# 
# That should be all you need to configure
#

OBJ= dtconf.o

H=
INC= 
RM= rm -f

all: $(OBJ) dtconf ../src/dtreg.h

../src/dtreg.h: dtconf.inf
	./dtconf

dtconf.o:$(H) dtconf.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c dtconf.c

dtconf:$(OBJ) dtconf.c
	$(CC) $(CFLAGS) -o $@ $(OBJ)
	./$@

clean:
	$(RM) *~ *.o core dtconf ../src/dtreg.h

build:
	make -i clean
	make all
