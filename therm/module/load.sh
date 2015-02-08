insmod /home/pi/rpi-scripts/therm_module/my_therm.ko
rm /dev/my_therm
mknod /dev/my_therm c 81 0
chmod 777 /dev/my_therm

