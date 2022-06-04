#!/bin/bash

hDir=`pwd`'/'

path=$(realpath "$1")
dir=$(dirname "$path")'/'
file=$(basename "$path")

cd $1
echo "imagedir="`pwd`"/"

mkdir -p ../logs
ls -1 | grep $file > ../logs/$file.log

cd ../logs/
echo "logpath="`pwd`"/"$file".log"

mkdir -p ../tmp/
cd ../tmp/
tmpdir=`pwd`"/"

# makes sure that you return to base directory
cd $hDir

./wrapper.py SuppressTourists $imagedir $tmpdir $logpath
