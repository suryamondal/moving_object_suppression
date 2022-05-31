#!/bin/bash

hDir=`pwd`'/'

# extracting frames from the viedo
while read line; do
    if [[ "$line" == *"FrameRate="* ]]; then
        framerate="${line/FrameRate=/""}"
    fi
    if [[ "$line" == *"FrameCount="* ]]; then
        framecount="${line/FrameCount=/""}"
    fi
    if [[ "$line" == *"FrameDir="* ]]; then
        framedir="${line/FrameDir=/""}"
    fi
    echo $line
done < <(./extract_frames.sh $1)
# echo 'FrameRate='$framerate
# echo 'FrameCount='$framecount
# echo 'FrameDir='$framedir

# makes sure that you return to base directory
cd $hDir

