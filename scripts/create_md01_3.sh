#!/bin/bash
mdadm -C /dev/md0 -l 5 -n 3 -c 4 /dev/sd{g,e,d}1 << EOF
yes
EOF

mdadm -C /dev/md1 -l 5 -n 3 -c 32 /dev/sd{g,e,d}2 << EOF
yes
EOF

