insmod /home/pi/rpi-scripts/lcd_module/char_dev.ko
rm /dev/char_dev
mknod /dev/char_dev c 223 0
chmod 777 /dev/char_dev
