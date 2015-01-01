#
# Compiling and Installing Isearch
#
# 1)  Type `make' 
#
# w)  Typing `make clean' will clean up .o files.
#
# The executables are placed in bin/<architecture>/
#
#       examplesearch - an example search program
#       odpsearch - a version of the Open Directory Search
#	Iindex	- command line indexing utility
#	Iutil - command line utilities for Isearch databases
#
SHELL=/bin/sh
OSNAME=`uname -s | tr "a-z" "A-Z"`

CC=g++

#
# Compiler flags
#
OPTIMISE_CFLAGS=-O2
DEBUG_CFLAGS=-O2
PROFILE_CFLAGS=-O2 -pg

# so the following will work for both Solaris and Linux.
#CFLAGS=-pg -O2 -DUNIX -D$(OSNAME)
CFLAGS=-O2 -DUNIX -D$(OSNAME)

#LDFLAGS=-pg
LDFLAGS=


INSTALL=/gh/bin
CGI_INSTALL=/gh/cgi-bin

TBIN_DIR=bin/$(OSNAME)
BIN_DIR=$(TBIN_DIR)

DOCTYPE_DIR=doctype
DOCLIB=

SRC_DIR=src

CGI_DIR=cgi-bin

TAR_DIR=../dist

# 
# That should be all you need to configure
#

RM = rm -f
VER=4.0
DIST=Isearch
BINDIST=$(DIST)-bin
freezename=`echo $(VER) | sed 's/\./-/g'`
OSVER=`uname -r`
PWD=`cd ..; pwd`
OS=$(OSNAME)_$(OSVER)
DIR=$(PWD)

all: setup isearch cgi-bin done

isearch::
	`if [ ! -f src/conf.h ] ; \
		then echo ./configure ; \
	fi`
	cd $(DOCTYPE_DIR); make "CC=$(CC)" \
		CFLAGS="$(CFLAGS) -DVERS=\\\"$(VER)\\\" "
	cd $(SRC_DIR); make "BIN_DIR=../$(BIN_DIR)" \
			"DOCTYPE_DIR=../$(DOCTYPE_DIR)" \
			CFLAGS="$(CFLAGS) -DVERS=\\\"$(VER)\\\" " \
			"CC=$(CC)" "DOCLIB=$(DOCLIB)" "LDFLAGS=$(LDFLAGS)"

cgi-bin::
	cd $(CGI_DIR); make "BIN_DIR=../$(BIN_DIR)" "VER=$(VER)" \
			"LIB_DIR=../$(BIN_DIR)" "ISEARCH_DIR=.." \
			CFLAGS="$(CFLAGS) -DTEMPLATE_DIR=\\\"$(DIR)/templates/\\\"\
			-DSEARCH_DB_DIR=\\\"$(DIR)/example/\\\"\
			-DBIASLIST_PATH=\\\"$(DIR)/catbias/\\\"\
			-DVERS=\\\"$(VER)\\\""\
			"CC=$(CC)" "DOCLIB=$(DOCLIB)" "LDFLAGS=$(LDFLAGS)"

bindir:
	@if [ ! -d bin ] ; then \
		mkdir bin ;\
		chmod 755 bin ;\
	fi
	@if [ ! -d bin/LINUX ] ; then \
		mkdir bin/LINUX ;\
		chmod 755 bin/LINUX ;\
	fi
	@if [ ! -d bin/SUNOS ] ; then \
		mkdir bin/SUNOS ;\
		chmod 755 bin/SUNOS ;\
	fi

setup: bindir
	@echo Architecute is $(OSNAME)
	@echo CFLAGS = $(CFLAGS)

build-test: 
	@if [ ! -f example/example.inx ] ; then \
		cd example;\
		../bin/$(OSNAME)/Iindex -d example_categories -t SIMPLE -f example_categories.fl ;\
		ln -s example.pn example_categories.pn ;\
		../bin/$(OSNAME)/Iindex -d example -t SITEDOC -f example.fl ;\
	fi

test: build-test
	(cd example; ../bin/$(OSNAME)/odp_search dmoz)

done:
	@echo ""
	@echo " Done building Isearch for architecture $(ARCH)"
	@echo ""

clean:
	@echo bindir = $(BIN_DIR)
	@echo osname = $(OSNAME)
	$(RM) *~ $(BIN_DIR)/Iindex $(BIN_DIR)/Isearch $(BIN_DIR)/Iutil \
		$(BIN_DIR)/libIsearch.a $(BIN_DIR)/odpsearch $(BIN_DIR)/examplesearch 
	cd $(SRC_DIR); make -i clean
	cd $(DOCTYPE_DIR); make -i clean
	cd $(CGI_DIR); make -i clean
	cd example; rm *.inx *.doctype *.pn *.fn *.mdt

install:
	@echo "make install does nothting."

