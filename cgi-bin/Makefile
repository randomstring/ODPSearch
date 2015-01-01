ARCH=`uname|tr "a-z" "A-Z"`
CC=g++

#
# If you have installed the Gnu iconv library uncomment the following 
# 2 lines
#LIBICONV=-liconv
#USEICONV=-DUSE_ICONV

#CFLAGS=-pg -Wall
#CFLAGS=-O2 -W -DUNIX
CFLAGS=-O2 -W -DUNIX $(USEICONV)

# cgi-bin Directory
CGIBIN_PATH=/gh/cgi-bin

INC= -I../doctype -I../src
LDFLAGS=
LIB=-lIsearch -lm $(LIBICONV)
COREOBJ=cgi-util.o search.o
OBJ=jdisp3_common.o $(COREOBJ)
H=newhoo.h demote_url.h cgi-util.h jdisp3.h

all: odp_search examplesearch done 

examplesearch: $(H) example-cgi.o $(LIB_DIR)/$(ISEARCH_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/$(ARCH)/$@ cgi-util.o example-cgi.o \
	        -L$(LIB_DIR) $(LIB)

odp_search: $(H) odp_search.o demote_url.o html.o jdisp3.o $(OBJ) $(LIB_DIR)/$(ISEARCH_LIB)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ../bin/$(ARCH)/$@ odp_search.o \
	        demote_url.o html.o jdisp3.o $(OBJ) -L$(LIB_DIR) -L../libiconv/lib $(LIB)

example-cgi.o: example-cgi.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c example-cgi.c

odp_search.o:odp_search.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c odp_search.c

old_odp_search.o:old_odp_search.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c old_odp_search.c

cgi-util.o: cgi-util.h cgi-util.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c cgi-util.c

html.o: html.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c html.c

search.o: search.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c search.c

jdisp3_common.o: jdisp3_common.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c jdisp3_common.c

jdisp3.o: jdisp3.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c jdisp3.c

shop_jdisp3.o: shop_jdisp3.c $(H)
	$(CC) $(CFLAGS) $(INC) -o $@ -c shop_jdisp3.c

%.o: $(H) %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $*.c

$(ISEARCH_LIB):$(LIB_DIR)/$(ISEARCH_LIB)
	@echo ""

done:
	@echo "Done building CGI."

clean:
	rm -f *.o *~ ../bin/$(ARCH)/*search 

install:
	@echo "Not supported."
	@echo ""
