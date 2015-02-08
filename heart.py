#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import re 
import os
import json
import argparse
import requests
import logging
import Adafruit_BMP.BMP085 as BMP085

logger = logging.getLogger("Heart")

OUTSIDE_TEMP_ID = "28-0000032f098e" 
INSIDE_TEMP_ID = "28-0000017815da" 
OUTSIDE_TABLE_ID = 2 
INSIDE_TABLE_ID = 1
PRESSURE_TABLE_ID = "pressure"
PUT_MEASURE_URL = "http://mbadowski.pl/househeart.pl/utils/dbUtils/putMeasure.php"
GET_MEASURE_URL = "http://mbadowski.pl/househeart.pl/utils/dbUtils/getMeasure.php"


def get_temp(temp_id):
    file_path = "/sys/bus/w1/devices/" + temp_id + "/w1_slave"
    try:
        logger.debug("Opening file: {}".format(file_path))
        file = open(file_path, "r")
        output = file.read()
        logger.debug("\n" + output)
        file.close()
        match = re.search("t=(-?\d*)", output)
        if match:
            temp = float(match.group(1))/1000
            logger.debug("Matched: {}".format(match.group(1)))
            return temp
        else:
            logger.error("Temperature cannot be parsed...")
    except IOError as err:
        logger.error("File not found...")
    return None

def get_humidity():
    file_path = "/dev/dht11"
    try:
        logger.debug("Opening file: {}".format(file_path))
        file = open(file_path, "r")
        output = file.read()
        logger.debug("\n" + output)
        file.close()
        match = re.search(".*, .*,.*,.*, (\d+),.* ", output)
        if match:
            hum = match.group(1)
            logger.debug("Matched: {}".format(match.group(1)))
            return int(hum)
        else:
            logger.error("Humidity check failed")
    except IOError as err:
        logger.error("File not found...")
    return None

def get_pressure():
    pressure_sensor = BMP085.BMP085()
    pressure_in_Pa = pressure_sensor.read_pressure()
    return pressure_in_Pa

def get_module_list():
    try:
        file = open("/proc/modules", "r")
        modules = file.read()
        file.close()
        return modules
    except IOError as err:
        logger.error("Modules file not found...")
    return None

def check_therm_module():
    modules = get_module_list()
    
    gpio_loaded = False
    therm_loaded = False

    if modules:
        if re.search("w1_gpio", modules):
            gpio_loaded = True
        else:
            print("Enabling 1-Wire...")
            os.system("sudo modprobe w1-gpio")
            if re.search("w1_gpio", modules):
                gpio_loaded = True

        
        if re.search("w1_therm", modules):
            therm_loaded = True
        else:
            print("Enabling 1w therm...")
            os.system("sudo modprobe w1-therm")
            if re.search("w1_therm", modules):
                therm_loaded = True

    return therm_loaded and gpio_loaded

def check_lcd_module():
    modules = get_module_list()
    if modules:
        if re.search("char_dev", modules):
            return True
        else:
            print("Loading char_dev...")
            os.system("sudo sh /home/pi/rpi-scripts/lcd/module/load.sh")
            if re.search("char_dev", modules):
                return True
    return False

def check_humidity_module():
    modules = get_module_list()
    if modules:
        if re.search("dht11km", modules):
            return True
        else:
            print("Loading dht11km...")
            os.system("sudo sh /home/pi/rpi-scripts/humidity/module/load.sh")
            if re.search("dht11km", modules):
                return True
    return False

def putMeasure(id, value):
    params = { 'id': id, 'value': value }
    r = requests.post(PUT_MEASURE_URL, data=params)
    return r.text

def getMeasure(temp_id, mode="last"):
    params = {'id': temp_id, 'mode': mode}
    r = requests.get(GET_MEASURE_URL, params=params)
    return r.text




if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--publish", "-p",
                        help="Flag which decide if measurements will be sent do database.",
                        default=False,
                        action="store_true")
    parser.add_argument("--print-lcd", "-l",
                        help="Decide if measurements shuld be printed on lcd device",
                        default=False,
                        action="store_true",
                        dest="print_lcd")
    parser.add_argument("--store-local", "-s",
                        help="Flag which decide if measurements will be stored in local file",
                        default=False,
                        action="store_true",
                        dest="store_local")
    parser.add_argument('-q', '--quiet', help='set logging to ERROR',
                        action='store_const', dest='loglevel',
                        const=logging.ERROR, default=logging.INFO)
    parser.add_argument('-d', '--debug', help='set logging to DEBUG',
                        action='store_const', dest='loglevel',
                        const=logging.DEBUG, default=logging.INFO)
    parser.add_argument('-v', '--verbose', help='set logging to COMM',
                        action='store_const', dest='loglevel',
                        const=5, default=logging.INFO)
    args = parser.parse_args()
    # Setup logging.
    logging.basicConfig(#filename='/home/pi/rpi-scripts/xmpp/server_thread.log',
                        #level=opts.loglevel,
                        level=args.loglevel,
                        format='%(levelname)-8s %(message)s')
    
    measurements = {"humidity": get_humidity(),
                    "pressure": get_pressure()}
    if check_therm_module():
        measurements["temperature_inside"] = get_temp(INSIDE_TEMP_ID)
        measurements["temperature_outside"] = get_temp(OUTSIDE_TEMP_ID)
        if args.publish:
            putMeasure(OUTSIDE_TABLE_ID, outsideTemp_str)
            putMeasure(INSIDE_TABLE_ID, insideTemp_str)
            putMeasure(PRESSURE_TABLE_ID, pressure_in_hPa)
    else:
        outsideTemp_str = "Err"
        insideTemp_str = "Err"
    
    measurements["pressure"] = get_pressure()

    if check_humidity_module():
        measurements["humidity"] = get_humidity()
    else:
        humidity = "Err"


    logger.info("Ostatnia temperatura na zawnatrz: " + ("%.2f" % float(getMeasure(OUTSIDE_TABLE_ID, "last"))))
    logger.info("Ostatnia temperatura wewnatrz: " + ("%.2f" % float(getMeasure(INSIDE_TABLE_ID, "last"))))
    logger.info("Aktualna na zewnatrz: " + ("%.1f" % measurements["temperature_outside"]))
    logger.info("Aktualna wewnatrz: " + ("%.1f" % measurements["temperature_inside"]))
    logger.info("Cisnienie: " + ("%.0f" % (measurements["pressure"]/100.0)))
    logger.info("Wilgotnosc: " + str(measurements["humidity"]))
    
    if args.print_lcd:
        formated_temp_in = "W:" + ("%.1f" % measurements["temperature_outside"]) + chr(223) + "C"
        formated_temp_out = "Z:" + ("%.1f" % measurements["temperature_inside"]) + chr(223) + "C"
        formated_pressure_hPa = "P:" + ("%.0f" % (measurements["pressure"]/100.0)) + "hPa"
        formated_humidity = "H:" + str(measurements["humidity"]) + "%"

        chars_left_1st_line = 16 - (len(formated_temp_in) + len(formated_temp_out))
        chars_left_2nd_line = 16 - (len(formated_pressure_hPa) + len(formated_humidity))

        for x in range(0, chars_left_1st_line):
            formated_temp_in += " "
        for x in range(0, chars_left_2nd_line):
            formated_pressure_hPa += " "

        first_line = formated_temp_in + formated_temp_out
        second_line = formated_pressure_hPa + formated_humidity

        os.system("echo " + first_line + second_line + " > /dev/char_dev")

    if args.store_local:
        file_path = "/home/pi/rpi-scripts/measurements.json"
        logger.debug("Storing to local file: " +  file_path)
        with open(file_path, 'w') as f:
            json.dump({"measurements": measurements}, f, ensure_ascii=False, indent = 4)
