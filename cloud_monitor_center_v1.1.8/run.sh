#!/bin/sh

echo "--------------begin to delete!--------------"
rm -rf cloud_monitor_center
cd ./src
rm -rf *.o
rm -rf cloud_monitor_center
echo "--------------delete complete!--------------"
sleep 1
cd ..
echo "--------------begin to make!--------------"
make
echo "--------------make complete!--------------"
sleep 1
echo "--------------start to running!!!--------------"
./cloud_monitor_center -c ./config/cloud_monitor_center.conf
