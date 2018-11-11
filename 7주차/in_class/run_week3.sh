#!/bin/sh

./waf --run "scratch/fifth" > fixed_udp_10000.dat 2>&1
./waf --run "scratch/fifth -rcv_buf=1500" > fixed_udp_1500.dat 2>&1
./waf --run "scratch/fifth -udp_rate=1Mbps" > fixed_recv_buf_1.dat 2>&1
./waf --run "scratch/fifth -udp_rate=3Mbps" > fixed_recv_buf_3.dat 2>&1
./waf --run "scratch/fifth -udp_rate=5Mbps" > fixed_recv_buf_5.dat 2>&1
