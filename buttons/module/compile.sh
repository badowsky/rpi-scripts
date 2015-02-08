#!/bin/bash 
git pull
sudo make -C /lib/modules/$(uname -r)/build M=$(pwd) modules
rmmod switch_module
insmod switch_module.ko
