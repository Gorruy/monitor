#!/usr/bin/bash

MY_IP=$(hostname -I | awk '{print $1}');
export PID;

# load test for profiling
# sudo perf record -g -F 100 -- ../bin/sniffer --interface lo & \
# sudo ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005 & \
# sleep 20; sudo ../bin/representer;
# sudo perf script | stackcollapse-perf.pl > out.perf-folded
# flamegraph.pl out.perf-folded > perf.svg

# Tests to check options:
sudo ../bin/sniffer --interface lo --ipdest ${MY_IP} --portdest 1005;

sleep 1;

echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;

sudo ../bin/representer;

