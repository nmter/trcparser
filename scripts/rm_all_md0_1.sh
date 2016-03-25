#!/bin/bash
dev0="/dev/md0"
dev1="/dev/md1"
ls $dev0 > /dev/null 2>&1
if [ $? == 0 ]; then
	#stop dev0
	mdadm --stop ${dev0}
	mdadm --zero-superblock /dev/sd{g,e,d,c,b}1
fi
ls $dev1 > /dev/null 2>&1
if [ $? == 0 ]; then
	#stop dev1
        mdadm --stop ${dev1}
        mdadm --zero-superblock /dev/sd{g,e,d,c,b}2
fi
