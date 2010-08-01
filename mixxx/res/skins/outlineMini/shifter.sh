#!/bin/bash
# GED's mixxx skin control shifter script file.
IFS='
'

function shiftx() {
	source=$1
	x_shift=$2
	x_threshold=$3
	x_threshold_stop=$4

	for x in $(grep '\<Pos\>' $source | cut -d, -f1 | cut -d\> -f2 | sort -rnu); do 
		# if [ $x -ge $x_threshold ] && ([ -z "$x_threshold_stop" ] || [ $x -lt $x_threshold_stop ]); then
		if [ -z "$x_threshold_stop" ] || [ $x -lt "$x_threshold_stop" ] && [ $x -ge $x_threshold ]; then 
			echo x $x \-\> `expr $x + $x_shift`
			sed -e "s/>$x,/>~`expr $x + $x_shift`,/g" $source > $source.tmp && mv $source.tmp $source
		else 
			echo x value of $x is not in $x_threshold to $x_threshold_stop range, skipping...
		fi
	done
}

function shifty() {
	source=$1
	y_shift=$2
	y_threshold=$3
	y_threshold_stop=$4
	
	for y in $(grep '</Pos>' skin.xml | cut -d, -f2 | cut -d\< -f1 | sort -rnu); do 		
#		if [ $y -lt $y_threshold_stop ]; then
#			break;
#		fi
		# if [ $y -ge $y_threshold ] && [[ -z "$y_threshold_stop" ] || [ $y -lt "$y_threshold_stop" ]];then
		if [ -z "$y_threshold_stop" ] || [ $y -lt "$y_threshold_stop" ] && [ $y -ge $y_threshold ]; then 
		# if [ $y -gt $y_threshold ]; then
			echo y $y \-\> `expr $y + $y_shift`
			sed -e "s/,$y</,~`expr $y + $y_shift`</g" $source > $source.tmp && mv $source.tmp $source
		else 
#			echo y value of $y is under $y_threshold, skipping...
			echo y value of $y is not in $y_threshold to $y_threshold_stop range, skipping...
		fi
	done
}

source=skin.xml
source_backup=$source.`date "+%Y-%m-%d-%H%S"`

if [ -z "$1" ] || [ "echo xy | grep $1 | wc -l" = 0 ]; then
	echo "Usage: $0 x/y shiftamount threshold_start [threshold_stop]"
	exit
fi

echo shifting $1
cp $source $source_backup # backup original

if [ $1 = "x" ]; then 
	shiftx $source $2 $3 $4
	sed -e "s/>~/>/g" $source > $source.tmp && mv $source.tmp $source # remove all the subsitution tags
fi
if [ $1 = "y" ]; then
	shifty $source $2 $3 $4
	sed -e "s/,~/,/g" $source > $source.tmp && mv $source.tmp $source # remove all the subsitution tags
fi

echo undo command is: 
echo
echo "    mv $source_backup $source"
echo
