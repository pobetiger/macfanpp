#!/bin/sh

while true; do
    printf "\33[2J\33[1;1f\n"
    echo "*** CPU ***"
    ./macfanpp | egrep -i "cpu"
    echo "*** GPU ***"
    ./macfanpp | egrep -i "gpu"
    echo "*** FAN ***"
    ./macfanpp | egrep "Fan"

    sleep 2
done
