#!/bin/sh
# note. you need to have a 'plantuml' executable in path

hash plantuml 2>/dev/null  || { echo >&2 "I require plantuml in path"; exit 1; }

for f in *.uml
do    
    plantuml -v $f
done


