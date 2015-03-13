insmod /home/pi/rpi-scripts/humidity/module/dht11km.ko
rm /dev/dht11
mknod /dev/dht11 c 80 0

