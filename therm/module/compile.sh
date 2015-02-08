#!/bin/bash 
git pull
sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
rmmod my_therm
insmod my_therm.ko
rm /dev/my_therm
mknod /dev/my_therm c 81 0
chmod 777 /dev/my_therm

