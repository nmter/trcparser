#!/bin/bash
a=`cat /proc/mdstat |grep "="`
#echo $a
if [ "$a" = "" ];then
	echo "yes"
fi
