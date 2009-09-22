#!/bin/bash
#
#   Test sound file generator for Mixxx
#       created by Sean M. Pappalardo on 7/20/2009
#
#   This script generates sound files in all of the formats
#   Mixxx supports with all of the various sample sizes,
#   sample rates, and channel numbers
#
#   Pass it the "clean" argument to delete all the files it makes
#   Pass it the "table" argument to generate a table of all the file formats & types in Wiki syntax

formats=("wav" "mp3" "ogg" "flac")  # mp3 must come after wav because it converts the wavs
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
                
                # Remove files from formats for which sample size is irrelevant
                if  ( [ $format == "ogg" ] || [ $format == "mp3" ] ) && [ -e test${friendlyrate}k${friendlychannel}.${format} ]
                then 
                    echo "Removing test${friendlyrate}k${friendlychannel}.${format}"
                    rm test${friendlyrate}k${friendlychannel}.${format}
                    if [ $? -gt 0 ]
                    then
                        echo "Error #$?"
                        exit 1
                    fi
                fi
                
                for ssize in ${samplesizes[*]}
                do
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
    if [ -e 1kHzR440HzLReference_32i96kStereo.wav ]
    then
        echo "Removing reference file"
        rm 1kHzR440HzLReference_32i96kStereo.wav
    fi
    exit 0
fi

# Unpack reference file
if [ "$1" != "table" ]
then
    echo "Unpacking reference file..."
    bunzip2 -k 1kHzR440HzLReference_32i96kStereo.wav.bz2
    if [ $? -gt 1 ]
    then
        echo "Unpacking reference file failed, aborting"
        exit 1
    fi
fi

# Do the conversions/generate the table
for format in ${formats[*]}
do
    if [ "$1" == "table" ]
    then
        if [ $format == "ogg" ] || [ $format == "mp3" ]
        then
            friendlyformat="OGG Vorbis"
        fi
        if [ $format == "wav" ]
        then
            friendlyformat="WAVE/AIFF"
        fi
        if [ $format == "mp3" ]
        then
            friendlyformat="MP3"
        fi
        if [ $format == "flac" ]
        then
            friendlyformat="FLAC"
        fi
        echo
        echo "==== $friendlyformat ===="
        echo "^ Channels ^ Bit depth ^ Sample Rate ^ Does it work? ^"
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
        for ssize in ${samplesizes[*]}
        do
            # Hack because sox doesn't abort if the parameters are out of spec
            if [ $ssize -gt 24 ] && [ $format == "flac" ]
            then
                if [ "$1" != "table" ]
                then
                    echo "FLAC doesn't support ${ssize}-bit, skipping"
                fi
                    break
            fi
            
            # vorbis and MP3 don't use bit depth, so only run sox once per sample rate
            if [ $ssize != ${samplesizes[0]} ] && ( [ $format == "ogg" ] || [ $format == "mp3" ] )
            then
                break
            fi
            for rate in ${samplerates[*]}
            do 
                friendlyrate=`expr $rate / 1000`
                problem="false"
                
                # Hack because lame doesn't abort if the parameters are out of spec
                if [ $rate -gt 48000 ] && [ $format == "mp3" ]
                then
                    if [ "$1" != "table" ]
                    then
                        echo "MP3 doesn't support ${rate}Hz, skipping"
                    fi
                    problem="true"
                fi
                
                if [ "$1" == "table" ] && [ "$problem" != "true" ]
                then
                    if [ $format == "ogg" ] || [ $format == "mp3" ]
                    then    # sample size is irrelevant for MP3 and Vorbis
                        if [ $channel -eq 1 ]
                        then
                            echo "^ Mono   ^        ^ $rate Hz |    |"
                        else
                            echo "^ $friendlychannel ^        ^ $rate Hz |    |"
                        fi
                    else
                        if [ $channel -eq 1 ]
                        then
                            echo "^ Mono   ^ $ssize-bit ^ $rate Hz |    |"
                        else
                            echo "^ $friendlychannel ^ $ssize-bit ^ $rate Hz |    |"
                        fi
                    fi
                fi
                
                if [ "$problem" != "true" ] && ( [ $format == "ogg" ] || [ $format == "mp3" ] )
                then
                    if [ "$1" != "table" ]
                    then
                        echo "Generating ${rate}Hz ${channel}-channel ${format} file"
                        if [ $format == "mp3" ]
                        then    # sox can't make MP3s by default, so we use LAME
                            lame -S $lameopt --tt test${friendlyrate}k${friendlychannel} test32bit${friendlyrate}k${friendlychannel}.wav test${friendlyrate}k${friendlychannel}.mp3
                        else    # For formats where sample size is irrelevant
                            sox -V0 1kHzR440HzLReference_32i96kStereo.wav -c ${channel} -r ${rate} test${friendlyrate}k${friendlychannel}.${format}
                        fi
                        problem="true"
                    fi
                fi
                
                # Use sox by default
                if [ "$1" != "table" ] && [ $problem != "true" ]
                then
                    echo "Generating ${ssize}-bit ${rate}Hz ${channel}-channel ${format} file"
                    sox -V0 1kHzR440HzLReference_32i96kStereo.wav -b ${ssize} -c ${channel} -r ${rate} test${ssize}bit${friendlyrate}k${friendlychannel}.${format}
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
echo
echo "Finished"
exit 0