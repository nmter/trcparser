#!/bin/bash
#examples:
#	Single CS for each part of .		MultiCS ...
#	./make_partition.sh 0 10%  20GB		./make_partition.sh 1 5% 10%  10GB+10GB 
#								      3% 10%	different ratio of R/W area.
#								      7% 10%
#	./make_partition.sh 0 20%  40GB		./make_partition.sh 1 10% 20% 20GB+20GB
#								      8%  20%
#								      12% 20%
#	./make_partition.sh 0 40%  80GB		./make_partition.sh 1 20% 40% 40GB+40GB
#								      	

mode=$1 #0 - Single CS partitions, 1 - Multi CS partition
Size=$2 #md0 part size eg. %10 = 200GB * %10 = 20GB
Size1=$3 #md1 part size 
if [ $mode == 0 ];then
	#Single CS
	for i in g e d b c
	do
		parted -s /dev/sd${i} mkpart primary 0% $Size
	done
else
	#Multi CS
	for i in g e d b c
        do
                parted -s /dev/sd${i} mkpart primary 0% $Size
		parted -s /dev/sd${i} mkpart primary $Size $Size1
        done

fi
