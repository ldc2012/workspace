#!/bin/sh

echo "--------------begin to delete!--------------"
rm -rf cloud_evaluation_center
cd ./src
rm -rf *.o
rm -rf cloud_evaluation_center
echo "--------------delete complete!--------------"
sleep 1
cd ..
echo "--------------begin to make!--------------"
make
echo "--------------make complete!--------------"
sleep 1
echo "--------------start to running!!!--------------"
./cloud_evaluation_center -c ./config/cloud_evaluation_center.conf
