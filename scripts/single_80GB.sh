#!/bin/bash
cs=$1 #4 or 32, means 4KB-chunk or 32KB-chunk
ssds=$2 #3,4,5 
sudo ./rm_all_md0_1.sh
sudo ./rm_all_partition.sh
sudo ./make_partition.sh 0 40% #80GB

case ${ssds} in
	3 )
		sudo ./create_md0_3.sh $cs
		;;
	4 )
		sudo ./create_md0_4.sh $cs
		;;
	5 )
		sudo ./create_md0_5.sh $cs
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

