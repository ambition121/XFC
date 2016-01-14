#!/bin/sh
#ifconfig | grep "TX bytes" | head -1 |awk -F ":" '{print $3}' | awk -F" " '{print $1}'
cat /sys/class/net/eth0/statistics/tx_bytes
