#!/bin/bash 
git pull
sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
rmmod char_dev
insmod char_dev.ko
rm /dev/char_dev
mknod /dev/char_dev c 223 0
chmod 777 /dev/char_dev

