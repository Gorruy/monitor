#!/usr/bin/bash

MY_IP=$(hostname -I | awk '{print $1}');

#!/usr/bin/bash

MY_IP=$(hostname -I | awk '{print $1}');

# load test for profiling
# sudo perf record -g -F 103 -- ../bin/sniffer --interface lo & \
# sudo offcputime-bpfcc -df -p `pgrep -nx sniffer` 20 > out.stacks & \
# sudo ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005 & \
# sleep 20; sudo ../bin/representer;
# sudo perf script | stackcollapse-perf.pl > out.perf-folded
# flamegraph.pl --title="CPU Time Flame Graph" out.perf-folded > perf.svg
# flamegraph.pl --color=io --title="Off-CPU Time Flame Graph" --countname=us < out.stacks > out.svg

# Tests to check ipdest:
sudo ./bin/sniffer --interface lo --ipdest ${MY_IP} > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ./bin/representer)" 

if [[ $res != "Number of packets:1, size of all packets in bytes:56" ]]; then
    echo "Error while checking ipdest!"
fi

sudo ./bin/sniffer --interface lo --ipdest 100.100.100.100 > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ./bin/representer)" 

if [[ $res != "Number of packets:0, size of all packets in bytes:0" ]]; then
    echo "Error while checking ipdest!"
fi

# Tests to check portdest:
sudo ./bin/sniffer --interface lo --portdest 1005 > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ./bin/representer)" 

if [[ $res != "Number of packets:1, size of all packets in bytes:56" ]]; then
    echo "Error while checking portdest!";
fi

sudo ./bin/sniffer --interface lo --portdest 123 > /dev/null & sleep 1;
echo "IP DEST TEST:" | nc -u -w0 ${MY_IP} 1005;
res="$(sudo ./bin/representer)" 

if [[ $res != "Number of packets:0, size of all packets in bytes:0" ]]; then
    echo "Error while checking portdest!"
fi

sudo ./bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005;

echo "Tests is over!"