import logging
import subprocess
import re

logger = logging.getLogger(__name__)

class MsgParser:

    def __init__(self):
        self.commands_map = {"temp": "/home/pi/rpi-scripts/temp/read_temp.py"}
        logger.info("Parser created.")

    def process(self, msg):
        logger.debug("Processing msg: " + msg)
        for key, value in self.commands_map.items():
            match = re.search(key, msg)
            if match:
                p = subprocess.Popen(['python3', value],
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
                output, error = p.communicate()
                logger.debug("Message processed, returning: \n" + output.decode())
                return output.decode()

        

