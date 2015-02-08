insmod /home/pi/rpi-scripts/hum_module/dht11km.ko
rm /dev/dht11
mknod /dev/dht11 c 80 0

