#!/bin/sh
#
# usage: build_odp_index 
#

if [ -r "content.rdf" ] ; then
    print "ERROR: can't find file list: content.rdf"
    print "Usage: build_index"
    exit 1
fi

echo "Generating BFB and filelists for indexing"
#rdf2bfb

echo "Generating Category Search Index"
Iindex -d category -t SIMPLE -f category.filelist

# This trick forces both the category and site database
# to use the same hash table for category (directory) names.
# This allows fast comparisons of category names, and saves
# disk space.
ln -s category.pn site.pn

echo "Generating Site Search Index"
Iindex -d site -t SITEDOC -f site.filelist

