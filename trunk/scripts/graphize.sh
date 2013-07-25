#!/bin/sh +x

FILENAME=$1
GENERATOR=$2 
OUTFILE=$FILENAME".gv"

# SED=D:\\apps\\gnuwin32\\bin\\sed.exe
SED=sed

GRAFNAME=`echo $FILENAME|$SED -e's/\..*//g'|$SED -e's/.*\///g'`

echo $GRAFNAME

echo "graph \"ab\" {" > $OUTFILE
echo "node [ " >> $OUTFILE
echo " fontname = \"Courier\"" >> $OUTFILE
echo " label = \"\\N\"" >> $OUTFILE
echo " shape = \"circle\"" >> $OUTFILE
echo " width = \"0.30000\"" >> $OUTFILE
echo " height = \"0.300000\"" >> $OUTFILE
echo " color = \"black\"" >> $OUTFILE
echo " ]" >> $OUTFILE
echo " edge [" >> $OUTFILE
echo " color = \"red\" " >> $OUTFILE
echo " ]" >> $OUTFILE

DATA=`cat $FILENAME|$SED -e 's/[[:space:]]*//g'|$SED -e 's/^#.*//g'|$SED -e 's/\(.*\),\(.*\)/\"\1\"--\"\2\" /g'`
echo $DATA ";}" >> $OUTFILE

if [[ $GENERATOR == "" ]]
then
    GENERATOR=neato
fi

$GENERATOR -Tdot -o$FILENAME.dot -K$GENERATOR $OUTFILE 
dotty.exe $FILENAME.dot 

