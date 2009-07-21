#!/bin/bash
#
#   Test sound file generator for Mixxx
#       created by Sean M. Pappalardo on 7/20/2009
#
#   This script generates sound files in all of the formats
#   Mixxx supports with all of the various sample sizes,
#   sample rates, and channel numbers
#

formats=("mp3")  # mp3 must come after wav
channels=(1 2)
samplesizes=(16 24 32)
samplerates=(22050 32000 44100 48000 96000)

if [ "$1" == "clean" ]
then
    for format in ${formats[*]}
    do
        for rate in ${samplerates[*]}
        do
            friendlyrate=`expr $rate / 1000`
            for ssize in ${samplesizes[*]}
            do
                for channel in ${channels[*]}
                do
                    if [ $channel -eq 1 ]
                    then
                        friendlychannel="Mono"
                    fi
                    if [ $channel -eq 2 ]
                    then
                        friendlychannel="Stereo"
                    fi
                    if [ -e test${ssize}bit${friendlyrate}k${friendlychannel}.${format} ]
                    then 
                        echo "Removing test${ssize}bit${friendlyrate}k${friendlychannel}.${format}"
                        rm test${ssize}bit${friendlyrate}k${friendlychannel}.${format}
                        if [ $? -gt 0 ]
                        then
                            echo "Error #$?"
                            exit 1
                        fi
                    fi
                done
            done
        done
    done
    if [ -e 1kHzReference_32f96kStereo.wav ]
    then
        echo "Removing reference file"
        rm 1kHzReference_32f96kStereo.wav
    fi
    exit 0
fi

echo "Unpacking reference file..."
bunzip2 -k 1kHzReference_32f96kStereo.wav.bz2
if [ $? -gt 1 ]
then
    echo "Unpacking reference file failed, aborting"
    exit 1
fi

for format in ${formats[*]}
do
    for rate in ${samplerates[*]}
    do 
        friendlyrate=`expr $rate / 1000`
        for ssize in ${samplesizes[*]}
        do
            # Hack because sox doesn't abort if the parameters are out of spec
            if [ $ssize == 32 ] && [ $format == "flac" ]
            then
                echo "FLAC doesn't support 32-bit, skipping"
                break
            fi
            # vorbis doesn't use bit depth, so only run sox once per sample rate
            if [ $ssize != ${samplesizes[0]} ] && [ $format == "ogg" ]
            then
                echo "Only generating vorbis files once, skipping"
                break
            fi
            for channel in ${channels[*]}
            do
                if [ $channel -eq 1 ]
                then
                    friendlychannel="Mono"
                    lameopt="-m m"
                fi
                if [ $channel -eq 2 ]
                then
                    friendlychannel="Stereo"
                    lameopt=""
                fi
                echo "Generating ${ssize}-bit ${rate}Hz ${channel}-channel ${format} file"
                if [ $format == "mp3" ]
                then
                    lame -S $lameopt --tt test${ssize}bit${friendlyrate}k${friendlychannel} test${ssize}bit${friendlyrate}k${friendlychannel}.wav test${ssize}bit${friendlyrate}k${friendlychannel}.mp3
                else
                    sox -V0 1kHzReference_32f96kStereo.wav -b ${ssize} -c ${channel} -r ${rate} test${ssize}bit${friendlyrate}k${friendlychannel}.${format}
                fi
                if [ $? -gt 1 ]
                then
                    echo "Error #$?, aborting"
                    exit 1
                fi
            done
        done
    done
done