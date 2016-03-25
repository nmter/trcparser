#!/bin/bash
mdadm -C /dev/md0 -l 5 -n 5 -c 4 /dev/sd{g,e,d,b,c}1 << EOF
yes
EOF

mdadm -C /dev/md0 -l 5 -n 5 -c 4 /dev/sd{g,e,d,b,c}1 << EOF
yes
EOF


