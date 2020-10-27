#!/bin/bash

cd ~/../..
cd dev
sudo gpart create -s GPT ada1
sudo gpart add -t freebsd-ufs -a 1M ada1
sudo newfs -U /dev/ada1p1
mkdir /newdisk
sudo mount /dev/ada1p1 /newdisk

cd ..
mkdir dir1
mkdir dir2
cd dir1
touch foo.txt
echo "hello" >> foo.txt
cd ..
cd dir2
touch foo2.txt
echo "hello again" >>foo2.txt
# sudo bash -c 'echo "hello again" > foo2.txt'

cd ..
cd ..

sudo dd if=/dev/ada1p1 of=partition.img

sudo bash -c 'hexdump partition.img >partition.hex'



