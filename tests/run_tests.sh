#!/usr/bin/bash

MY_IP=$(hostname -I | awk '{print $1}');
export PID;

# load test for profiling
sudo perf record -g -F 100 ../bin/sniffer --interface lo; PID=$! & sudo netcat -kul 1005 & sudo ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005;

sudo kill -INT -$PID
sudo perf script > out.perf;

# Tests to check options:
# sudo ../bin/sniffer --interface lo --ipdest ${MY_IP} --portdest 1005 & sleep 1;

# echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;

# sudo ../bin/representer;