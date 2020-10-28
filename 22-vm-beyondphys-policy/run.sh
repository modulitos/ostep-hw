#!/bin/bash

policies=("FIFO" "LRU" "OPT" "UNOPT" "RAND" "CLOCK")
# policies=("UNOPT")

for policy in "${policies[@]}"
do
    for i in 1 2 3 4
    do
        ./paging-policy.py -c -f ./vpns.txt -p "$policy" -C "$i"
    done
    echo ""
done
