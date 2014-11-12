import re 
import os

outsideTempID = "28-0000032f098e" 
insideTempID = "28-0000017815da" 

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

#check_modules()
print("Temperature outside: " + str(check_temp(outsideTempID)) + "\xb0C")
print("Temperature inside: " + str(check_temp(insideTempID)) + "\xb0C")
