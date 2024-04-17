MY_IP=$(hostname -I | awk '{print $1}');

../bin/sniffer --interface lo & \
offcputime-bpfcc -df -p `pgrep -nx sniffer` 20 > out.stacks & \
timeout 20s ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005;
../bin/representer;
#flamegraph.pl --color=io --title="Off-CPU Time Flame Graph" --countname=us < out.stacks > out.svg;