MY_IP=$(hostname -I | awk '{print $1}');

sudo ../bin/sniffer --interface lo & \
sudo offcputime-bpfcc -df -p `pgrep -nx sniffer` 20 > out.stacks & \
sudo timeout 20s ../bin/tests --interface lo --ipdest ${MY_IP} --portdest 1005;
sudo ../bin/representer;
flamegraph.pl --color=io --title="Off-CPU Time Flame Graph" --countname=us < out.stacks > out.svg;
