#!/bin/bash
cs=$1
mdadm -C /dev/md0 -l 5 -n 4 -c ${cs} /dev/sd{g,e,d,b}1 << EOF
yes
EOF

