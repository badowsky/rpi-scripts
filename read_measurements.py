# -*- coding: utf-8 -*-
import re 
import os
import requests
import Adafruit_BMP.BMP085 as BMP085

OUTSIDE_TEMP_ID = "28-0000032f098e" 
INSIDE_TEMP_ID = "28-0000017815da" 
OUTSIDE_TABLE_ID = 2 
INSIDE_TABLE_ID = 1
PRESSURE_TABLE_ID = "pressure"
PUT_MEASURE_URL = "http://mbadowski.pl/househeart.pl/utils/dbUtils/putMeasure.php"
GET_MEASURE_URL = "http://mbadowski.pl/househeart.pl/utils/dbUtils/getMeasure.php"


def check_temp(temp_id):
	file = open("/sys/bus/w1/devices/" + temp_id + "/w1_slave", "r")
	output = file.read()
	file.close()
	match = re.search("t=(-?\d*)", output)
	if match:
		temp = float(match.group(1))/1000
		return temp
	else:
		return "Temperature not found." 

def check_hum():
        file = open("/dev/dht11", "r")
        output = file.read()
        file.close()
        match = re.search(".*, .*,.*,.*, (\d+),.* ", output)
        if match:
                hum = match.group(1)
                return hum
        else:
                return "Humidity check failed"


def check_modules():

        file = open("/proc/modules", "r")
        modules = file.read()
        file.close()

        if not re.search("w1_gpio", modules):
                print("Enabling 1-Wire...")
                os.system("sudo modprobe w1-gpio")
      
        if not re.search("w1_therm", modules):
                print("Enabling 1w therm...")
                os.system("sudo modprobe w1-therm")

        if not re.search("char_dev", modules):
                print("Loading char_dev...")
                os.system("sudo sh /home/pi/rpi-scripts/lcd_module/load.sh")

        if not re.search("dht11km", modules):
                print("Loading dht11km...")
                os.system("sudo sh /home/pi/rpi-scripts/hum_module/run.sh")

def putMeasure(id, value):
	params = { 'id': id, 'value': value }
	r = requests.post(PUT_MEASURE_URL, data=params)
	return r.text

def getMeasure(temp_id, mode="last"):
	params = {'id': temp_id, 'mode': mode}
	r = requests.get(GET_MEASURE_URL, params=params)
	return r.text


check_modules()
outsideTemp_str = ("%.1f" % check_temp(OUTSIDE_TEMP_ID))
insideTemp_str = ("%.1f" % check_temp(INSIDE_TEMP_ID))


#last_out_temp = ("%.2f" % float(getMeasure(OUTSIDE_TABLE_ID, "last")))
#last_in_temp = ("%.2f" % float(getMeasure(INSIDE_TABLE_ID, "last")))

#putMeasure(OUTSIDE_TABLE_ID, outsideTemp_str)
#putMeasure(INSIDE_TABLE_ID, insideTemp_str)


pressure_sensor = BMP085.BMP085()

pressure_in_Pa = pressure_sensor.read_pressure()
pressure_in_hPa = "%.0f" % (pressure_in_Pa/100.0)
putMeasure(PRESSURE_TABLE_ID, pressure_in_hPa)

humidity = check_hum()

#print("Ostatnia temperatura na zawnatrz: " + str(last_out_temp))
#print("Ostatnia temperatura wewnatrz: " + str(last_in_temp) )
print("Aktualna na zewnatrz: " + outsideTemp_str )#+ u'\N{DEGREE SIGN}' + "C")
print("Aktualna wewnatrz: " + insideTemp_str )#+ u'\N{DEGREE SIGN}' + "C")
print("Cisnienie: " + pressure_in_hPa + "hPa")
print("Wilgotnosc powietrza: " + humidity + "%")

formated_temp_in = "W:" + insideTemp_str + chr(223) + "C"
formated_temp_out = "Z:" + outsideTemp_str + chr(223) + "C"

chars_left_1st_line = 16 - (len(formated_temp_in) + len(formated_temp_out))

for x in range(0, chars_left_1st_line):
	formated_temp_in += " "

first_line = formated_temp_in + formated_temp_out

second_line = "P:" + pressure_in_hPa  + "hPa H:" + humidity + "%"
os.system("echo " + first_line + second_line + " > /dev/char_dev")
