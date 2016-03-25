#!/bin/bash
#cs=$1 #4 or 32, means 4KB-chunk or 32KB-chunk
ssds=$1 #3,4,5 
sudo ./rm_all_md0_1.sh
sudo ./rm_all_partition.sh

sudo ./make_partition.sh 1 24% 40% #80GB

case ${ssds} in
	3 )
		sudo ./create_md01_3.sh
		;;
	4 )
		sudo ./create_md01_4.sh
		;;
	5 )
		sudo ./create_md01_5.sh
		;;
esac

a=`./create_done.sh`
while [ "$a" != "yes" ]
do
        sleep 100
        #echo "not yes"
        a=`./create_done.sh`
done
echo "yes & test now"

