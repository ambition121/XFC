#!/bin/sh
pid=`ps aux | grep "root@112.74.89.131" | grep -v "S+" | awk -F" " '{print $2}'`
kill -9 $pid>/dev/null

