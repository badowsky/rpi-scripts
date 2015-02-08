#!/bin/bash 
git pull
sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
rmmod dht11km
insmod dht11km.ko format=3
rm /dev/dht11
mknod /dev/dht11 c 80 0
chmod 777 /dev/dht11

