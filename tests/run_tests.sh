#!/usr/bin/sh

MY_IP=$(hostname -I | awk '{print $1}');

sudo ../bin/sniffer --ipdest ${MY_IP} --portdest 1005 & sudo ./tests --ipdest ${MY_IP} --portdest 1005;

sudo ../bin/representer;

sudo ../bin/sniffer --ipdest ${MY_IP} --portdest 1005 & sleep 1;

echo "TEST_MESSAGE" | nc -u -w1 ${MY_IP} 1005;

sudo ../bin/representer;