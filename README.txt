
		      Open Directory/Dmoz Search

Quick Start

  For those who wish to skip ahead:

  make
  make test

Background

This is Dmoz Search, used to power http://search.dmoz.org/ and
http://search.netscape.com/. This search is loosely base on Isearch
(http://www.cnidr.org/ir/isearch.html), when I say loosely, I mean way
back in 1998 I downloaded Isearch and started hacking on it.  Now,
several years later the number of unchanged lines of code is pretty
close to 1%. Algorithms, data structures, and the fundamental
philosophy of search results has been changed. As a result, the Open
Directory Search is orders of magnitude faster, can handle much larger
databases and much larger result sets.

The original license for Isearch states that:

  "Users of this software agree to make their best efforts (a) to return
  to MCNC any improvements or extensions that they make, so that these may
  be included in future releases; and (b) to inform MCNC/CNIDR of noteworthy
  uses of this software."

The full text of the original Isearch license is available in the
COPYRIGHT.txt found in the tarball. 

Considering MCNC/CNIDR (Clearinghouse for Networked Information
Discovery and Retrieval) is no longer maintaining Isearch, releasing a
snapshot of the code under the Mozilla Public License seems the best
way to honor the original copyright.

Dmoz Search is not as generalized as Isearch originally was, 
lots of dead code had to be trimmed for the sake of efficiency.


Requirements

Your machine and memory requirements will vary depending on the size
of the data set you will be using Isearch with. If you plan to index
and search the Open Directory Content, you will need a machine with
at least 2 Gigabytes of RAM. ODPsearch uses mmap() to speed up
access to its database files, due to a limit of Linux and Solaris
no more than 2GB of RAM can be addressed by a single process. This
limits the total size of the databases to 2Gigabytes.


Directories

src        - source for core search functionality
doctype    - document type parsing and scoring 
cgi-bin    - source for CGI programs
catbias    - category bias files, category and locality bias lists
templates  - html header and footer templates
scripts    - scripts for generating the Search DBs from the ODP RDF file
example    - an example dataset, use "make test" to test


Building

Dmoz Search only works on Linux and Solaris. It may run on other 
flavors of UNIX, but I have not tested them. The original Isearch
ran on Windows as well, but its my belief that if you want fast
search, scalable search you won't be using Windows anyway. 

Compiling Dmoz search should be as easy as typing "make" in the top
level directory. Binaries will be deposited in the bin sub-directory
under the appropriate architecture.

The programs are:

Iindex        - the indexing program, this takes a preformatted data input 
	        file and generates the index files for searching.
Iutil         - for querying the search index, for checking index
	        integrity and checking record meta data
examplesearch - this is an example program for querying the search 
		database. The output is in HTML. This is very basic,
                but is provided as an example of how to get started.

odpsearch     - a version of the odpsearch, very similar to that used
	        on the Open Directory Site http://search.dmoz.org/

After you run make, you can test the installation with "make
test". This will attempt to build search DBs from the example data in
the example directory. Then it will run the CGI from the command line,
producing html.

Also included are some scripts for indexing the the Open Directory RDF
data dumps (http://dmoz.org/rdf/). They are located in the "scripts"
directory:

rdf2bfb     - this takes the Open Directory RDF and generates the necessary
              "bfb" (big frigg'n buffer) and virtual filename files needed
	      by Iindex.

build_index.sh  - this is a shell script that calls rdf2bfb and runs Iindex
              to generate the search files 

These scripts are just a starting point, they will need to be customized
for your local use. The target directories for the search DBs will need
to be coordinated with the search binaries. So that the CGIs know where
to look for the search DBs.

If you don't want to play with the whole Open Directory, I have placed
a subset of the ODP RDF in:
     http://dmoz.org/ODPSearch/Arts_Only.content.rdf.u8.gz 
This is a UTF-8 encoded RDF file that has been gzip'd.


Dmoz Search Internals

Dmoz search builds its index from two input files, say the name of the
database will be "example". Then the two files are example.bfb and
example.fl. The first is a list of all the data to be indexed. If
there are separate files or records they are separated by a null
character '\0'. The second file is a "file list" or rather a list of
virtual filenames, where the directory of the virtual filename is the
category that the file/record appears in. The indexer reads in the
list of virtual files and the null delimited records/files and indexes
them.

Indexing a record/file consist of 
  1. Recording record/file data that pertains to the whole file in 
     the example.mdt (mdt = multiple document table). 
  2. Adding a hash table entry for the filename and the directory
     (also known as the category). These files are example.fn
     and example.pn. These hashes can be shared between databases
     that have same same set of filename and/or directories. This
     saves space, and allows comparisons between two databases.
  3. A list of word offsets is created into the record/file. This
     is the offset of the start of the word within the the bfb file.
     These ultimately go into the example.inx file once they have
     been sorted.

Documents are parsed based on the parsers found in the respective
document type classes. The document types are found in the doctype
directory. Currently only simple text, colon delimited text, and
some variants of colon delimited text parsers are included. This
is what the ODP uses. Perl is used to pre-process the RDF from
the ODP into the colon delimited text that Iindex parses. Adding
new document types, with new parsers, scoring, and fields should
be pretty straight forward after looking at the code of the other 
document types. The parsers are somewhat complicated because they
understand UTF-8 and character boundaries in Chinese and Japanese.

The odpsearch binary has stubs to call iconv() that attempts to
convert the input search string and category to UTF-8 before
performing the search. Using the iconv() call requires gnu libiconv
libraries to be installed. This library can be found at
http://www.gnu.org/software/libiconv/. If you install liviconv you
will need to uncomment the appropriate lines in the cgi-bin/Makefile.

Everything else is well documented by the code itself. 

Bryn Dole
dole@netscape.com
May 1, 2002

