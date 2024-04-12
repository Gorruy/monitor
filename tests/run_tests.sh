#!/usr/bin/bash

MY_IP=$(hostname -I | awk '{print $1}');
export PID;

# load test for profiling
# sudo perf record -g -F 100 -- ../bin/sniffer --interface lo & \
# sudo ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005 & \
# sleep 20; sudo ../bin/representer;
# sudo perf script | stackcollapse-perf.pl > out.perf-folded
# flamegraph.pl out.perf-folded > perf.svg

# Tests to check ipdest:
sudo ../bin/sniffer --interface lo --ipdest ${MY_IP} > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ../bin/representer)" 

if [[ $res != "Number of packets:1, size of all packets in bytes:56" ]]; then
    echo "Error while checking ipdest!"
fi

sudo ../bin/sniffer --interface lo --ipdest 100.100.100.100 > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ../bin/representer)" 

if [[ $res != "Number of packets:0, size of all packets in bytes:0" ]]; then
    echo "Error while checking ipdest!"
fi

# Tests to check portdest:
sudo ../bin/sniffer --interface lo --portdest 1005 > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ../bin/representer)" 

if [[ $res != "Number of packets:1, size of all packets in bytes:56" ]]; then
    echo "Error while checking portdest!"
fi

sudo ../bin/sniffer --interface lo --portdest 123 > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ../bin/representer)" 

if [[ $res != "Number of packets:0, size of all packets in bytes:0" ]]; then
    echo "Error while checking portdest!"
fi
