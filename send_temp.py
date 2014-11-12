import re 
import os
import MySQLdb

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

check_modules()
outsideTemp_str = str(check_temp(outsideTempID))
insideTemp_str = str(check_temp(insideTempID))
con = MySQLdb.connect("mbadowski.pl", "badowsky_OWadmin", "ba1805di", "badowsky_house_one_wire")
query_out = 'INSERT into `2` (value) values (\"{tempOut}\")'.format(tempOut=outsideTemp_str)
query_in = 'INSERT into `1` (value) values (\"{tempIn}\")'.format(tempIn=insideTemp_str)
query_out_last = 'Select value from `2` where date = (Select MAX(date) from `2`)'
query_in_last = 'Select value from `1` where date = (Select MAX(date) from `1`)'

c = con.cursor()
c.execute(query_out_last)
last_out_temp = c.fetchall()[0][0]

c.execute(query_in_last)
last_in_temp = c.fetchall()[0][0]

print("Ostatnia temperatura na zawnatrz: " + str(last_out_temp) )
print("Ostatnia temperatura wewnatrz: " + str(last_in_temp) )
print("Aktualna na zewnatrz: " + outsideTemp_str )
print("Aktualna wewnatrz: " + insideTemp_str )


#if str(last_out_temp) != outsideTemp_str:
c.execute(query_out)
#	print("Wysylam zewnatrzna.")

#if str(last_in_temp) != insideTemp_str:
c.execute(query_in)
#	print("Wysylam wewnatrzna.")

