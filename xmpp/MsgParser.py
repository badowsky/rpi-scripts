import logging
import subprocess

logger = logging.getLogger(__name__)

class MsgParser:

    def __init__(self):
        self.commands_map = {"temp": "/home/pi/rpi-scripts/temp/read_temp.py"}
        logger.info("Parser created.")

    def process(self, msg):
        logger.debug("Processing msg: " + msg)
        for key, value in self.commands_map:
            if re.search(key, msg):
                p = subprocess.Popen(['python3', value],
                                     stdout=subprocess.PIPE,
                                     stderr=subprocess.PIPE)
                output, error = p.communicate()
                return output.decode()

        

