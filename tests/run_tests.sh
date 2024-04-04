
    MY_IP=$(hostname -I | awk '{print $1}');

sudo ../bin/sniffer --ipdest ${MY_IP} --portdest 1005 & sudo ./tests --ipdest ${MY_IP} --portdest 1005;

sudo ../bin/representer;