#!/bin/bash
mdadm -C /dev/md0 -l 5 -n 4 -c 4 /dev/sd{g,e,d,b}1 << EOF
yes
EOF

mdadm -C /dev/md1 -l 5 -n 4 -c 32 /dev/sd{g,e,d,b}1 << EOF
yes
EOF

