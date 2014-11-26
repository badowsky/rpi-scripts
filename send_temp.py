import re 
import os
import requests

OUTSIDE_TEMP_ID = "28-0000032f098e" 
INSIDE_TEMP_ID = "28-0000017815da" 
OUTSIDE_TABLE_ID = 2 
INSIDE_TABLE_ID = 1
PUT_TEMP_URL = "http://mbadowski.pl/ownet/db/dbUtils/putTemperature.php"
GET_TEMP_URL = "http://mbadowski.pl/ownet/db/dbUtils/getTemperature.php"


def check_temp(temp_id):
	file = open("/sys/bus/w1/devices/" + temp_id + "/w1_slave", "r")
	output = file.read()
	file.close()
	match = re.search("t=(\d*)", output)
	if match:
		temp = float(match.group(1))/1000
		return temp
	else:
		return "Temperature not found." 

def check_modules():
	if not os.path.exists("/sys/bus/w1/devices"):
		print("Enabling 1-Wire...")
		os.system("sudo modprobe w1-gpio")
		os.system("sudo modprobe w1-therm")

def putTemperature(temp_id, value):
	params = { 'id': temp_id, 'value': value }
	r = requests.post(PUT_TEMP_URL, data=params)
	return r.text

def getTemperature(temp_id, mode="last"):
	params = {'id': temp_id, 'mode': mode}
	r = requests.get(GET_TEMP_URL, params=params)
	return r.text



check_modules()
outsideTemp_str = ("%.1f" % check_temp(OUTSIDE_TEMP_ID))
insideTemp_str = ("%.1f" % check_temp(INSIDE_TEMP_ID))


last_out_temp = ("%.2f" % float(getTemperature(OUTSIDE_TABLE_ID, "last")))
last_in_temp = ("%.2f" % float(getTemperature(INSIDE_TABLE_ID, "last")))

putTemperature(OUTSIDE_TABLE_ID, outsideTemp_str)
putTemperature(INSIDE_TABLE_ID, insideTemp_str)

print("Ostatnia temperatura na zawnatrz: " + str(last_out_temp) )
print("Ostatnia temperatura wewnatrz: " + str(last_in_temp) )
print("Aktualna na zewnatrz: " + outsideTemp_str )
print("Aktualna wewnatrz: " + insideTemp_str )

os.system("echo \"" + "W:" + insideTemp_str + " C " + "Z:" + outsideTemp_str + " C"  + "\" > /dev/char_dev")
