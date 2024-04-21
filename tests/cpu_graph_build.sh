#!/usr/bin/bash

MY_IP=$(hostname -I | awk '{print $1}');

perf record -g -F 103 -- ../bin/sniffer --interface lo & \
timeout 20s ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005;
../bin/representer && sudo pkill -2 sniffer;
#sudo perf script | stackcollapse-perf.pl > out.perf-folded;
#flamegraph.pl --title="CPU Time Flame Graph" out.perf-folded > perf.svg;