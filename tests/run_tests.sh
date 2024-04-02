
MY_IP=$(hostname -i);

sudo ../bin/sniffer --ipdest "${MY_IP}" --portdest 12345

sudo ./tests --ipdest "${MY_IP}" --portdest 12345