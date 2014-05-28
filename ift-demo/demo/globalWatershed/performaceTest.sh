#!/bin/bash
n=$1
for i in `seq 1 $n`
do
    echo $i
    ./watershed flower.pgm flower_grad.pgm flower-seeds.txt  | grep "time: \([0-9.]*\)" -o | grep -o "[0-9.]*" >> timeResults
done
