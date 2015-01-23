import logging

logger = logging.getLogger(__name__)

class MsgParser:

    def __init__(self):
        logger.info("Parser created.")

    def process(self, msg):
        logger.debug("Processing msg: " + msg)

