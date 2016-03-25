#!/bin/bash
cs=$1
mdadm -C /dev/md0 -l 5 -n 5 -c ${cs} /dev/sd{g,e,d,b,c}1 << EOF
yes
EOF

