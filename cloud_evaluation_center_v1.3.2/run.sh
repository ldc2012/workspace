#!/bin/sh

echo "--------------begin to delete!--------------"
rm -rf evaluation
cd ./src
rm -rf *.o
rm -rf evaluation
echo "--------------delete complete!--------------"
sleep 1
cd ..
echo "--------------begin to make!--------------"
make
echo "--------------make complete!--------------"
sleep 1
echo "--------------start to running!!!--------------"
./evaluation -c ./config/evaluation.conf
