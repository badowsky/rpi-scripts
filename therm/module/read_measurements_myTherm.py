import re 
import os
import requests
import Adafruit_BMP.BMP085 as BMP085

OUTSIDE_TABLE_ID = 2 
INSIDE_TABLE_ID = 1
PRESSURE_TABLE_ID = "pressure"
PUT_MEASURE_URL = "http://mbadowski.pl/househeart.pl/utils/dbUtils/putMeasure.php"
GET_MEASURE_URL = "http://mbadowski.pl/househeart.pl/utils/dbUtils/getMeasure.php"


def check_temp():
	file = open("/dev/my_therm", "r")
	output = file.read()
	file.close()
	match = re.search("IN: (-?\d+)\nOUT: (-?\d+)", output)
	if match:
		temp_in = float(match.group(1))/1000
		temp_out = float(match.group(2))/1000
		return temp_in, temp_out
	else:
		return "Temperature measure filed." 

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

	if not re.search("char_dev", modules):
		print("Loading char_dev...")
		os.system("sudo sh /home/pi/rpi-scripts/lcd_module/load.sh")

	if not re.search("my_therm", modules):
		print("Loading my_therm...")
		os.system("sudo sh /home/pi/rpi-scripts/therm_module/load.sh")

def putMeasure(id, value):
	params = { 'id': id, 'value': value }
	r = requests.post(PUT_MEASURE_URL, data=params)
	return r.text

def getMeasure(temp_id, mode="last"):
	params = {'id': temp_id, 'mode': mode}
	r = requests.get(GET_MEASURE_URL, params=params)
	return r.text


check_modules()
temp_in, temp_out = check_temp()
outsideTemp_str = ("%.1f" % temp_out)
insideTemp_str = ("%.1f" % temp_in)


#last_out_temp = ("%.2f" % float(getMeasure(OUTSIDE_TABLE_ID, "last")))
#last_in_temp = ("%.2f" % float(getMeasure(INSIDE_TABLE_ID, "last")))

#putMeasure(OUTSIDE_TABLE_ID, outsideTemp_str)
#putMeasure(INSIDE_TABLE_ID, insideTemp_str)


pressure_sensor = BMP085.BMP085()

pressure_in_Pa = pressure_sensor.read_pressure()
pressure_in_hPa = "%.0f" % (pressure_in_Pa/100.0)
#putMeasure(PRESSURE_TABLE_ID, pressure_in_hPa)

humidity_string = check_hum()

#print("Ostatnia temperatura na zawnatrz: " + str(last_out_temp))
#print("Ostatnia temperatura wewnatrz: " + str(last_in_temp) )
print("Aktualna na zewnatrz: " + outsideTemp_str )
print("Aktualna wewnatrz: " + insideTemp_str )


#print(pressure_in_Pa)
print("Cisnienie: " + pressure_in_hPa + "hPa")
send_pressure_cmd = "echo \"P:{value}hPa\" > /dev/char_dev".format(value=pressure_in_hPa)
#print(send_pressure_cmd)

print("Wilgotnosc: " + humidity_string + "%")

formated_temp_in = "W:" + insideTemp_str + chr(223) + "C"
formated_temp_out = "Z:" + outsideTemp_str + chr(223) + "C"

chars_left_1st_line = 16 - (len(formated_temp_in) + len(formated_temp_out))

for x in range(0, chars_left_1st_line):
	formated_temp_in += " "

first_line = formated_temp_in + formated_temp_out

second_line = "P:" + pressure_in_hPa  + "hPa" + " H:" + humidity_string + "%"
os.system("echo " + first_line + second_line + " > /dev/char_dev")
