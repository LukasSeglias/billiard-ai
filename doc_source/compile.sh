#!/bin/sh

rm -f /interface/output/*

find /source -type f -print0 | xargs -0 dos2unix --
cd /source/${LANGUAGE}

for d in */ ; do
    mkdir /interface/output/$d
done

bfh_files=`ls /usr/share/texlive/texmf-dist/tex/latex/bfh/*`
for bfh_file in $bfh_files
do
   cp $bfh_file ./
done

{ # your 'try' block
    pdflatex -output-directory /interface/output document.tex &&
    biber /interface/output/document.bcf &&
    pdflatex -output-directory /interface/output document.tex &&
    pdflatex -output-directory /interface/output document.tex
} || { # your 'catch' block
    echo "Error"
}


for bfh_file in $bfh_files
do
   rm `basename "$bfh_file"`
done