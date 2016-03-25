#!/bin/bash
for i in g e d b c
do
	sudo parted -s /dev/sd${i} rm 1 > /dev/null
	sudo parted -s /dev/sd${i} rm 2 > /dev/null
done
