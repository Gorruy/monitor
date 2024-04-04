
MY_IP=$(hostname -I | awk '{print $1}');

sudo ../bin/sniffer --ipdest ${MY_IP} --portdest 12345 & sudo ./tests --ipdest ${MY_IP} --portdest 12345;

sudo ../bin/representer;