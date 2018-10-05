#!/bin/sh

make clean
make light-hello.upload login | tee /home/user/Desktop/result.txt
