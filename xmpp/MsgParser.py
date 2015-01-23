import logging
import subprocess
import re

logger = logging.getLogger(__name__)

class MsgParser:

    def __init__(self):
        self.commands_map = {"temp": {"cmd":"python3", "args":"/home/pi/rpi-scripts/temp/read_temp.py"},
                             "all": {"cmd":"python3", "args":"/home/pi/rpi-scripts/read_measurements.py"},
                             "reboot": {"cmd":"shutdown", "args":"-r now"}}
        logger.info("Parser created.")

    def process(self, msg):
        logger.debug("Processing msg: " + msg)
        for key, value in self.commands_map.items():
            match = re.search(key, msg.lower())
            if match:
                p = subprocess.Popen([value['cmd'], value['args']],
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
                output, error = p.communicate()
                logger.debug("Message processed, returning: \n" + output.decode())
                return output.decode()

        

          logger.debug("Message processed, returning: \n" + output.decode())
                return output.decode()

        

