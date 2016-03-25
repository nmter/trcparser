#!/bin/bash
cs=$1
mdadm -C /dev/md0 -l 5 -n 3 -c ${cs} /dev/sd{g,e,d}1 << EOF
yes
EOF

