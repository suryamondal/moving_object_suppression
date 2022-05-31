#!/bin/bash

hDir=`pwd`'/'

path=$(realpath "$1")
dir=$(dirname "$path")'/'
file=$(basename "$path")
filename="${file%.*}"
echo 'dir='$dir
echo 'file='$file
# echo 'name -> '$filename

cd $dir

mkdir -p ../frames
cd ../frames
echo "FrameDir="`pwd`"/"
cd $dir

framecount=$(mediainfo --Inform='Video;%FrameCount%' $file)
framerate=$(mediainfo --Inform='Video;%FrameRate%' $file)
echo 'FrameRate='$framerate
echo 'FrameCount='$framecount

exec='ffmpeg -i '$file' ../frames/'$filename'_%08d.jpg -hide_banner'
echo $exec
# echo $exec | sh

ls -1 ../frames/ | grep $filename > ../logs/$filename.log

cd $hDir
