#! /bin/bash

#can2
sudo ip link set can2 type can bitrate 500000 dbitrate 2000000 fd on
sudo ip link set up can2

#can3
sudo ip link set can3 down
sudo ip link set can3 type can bitrate 1000000
sudo ip link set can3 up

