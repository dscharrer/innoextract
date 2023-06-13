# Custom Keywords

class custom(object):
    ROBOT_LIBRARY_SCOPE = 'TEST'

    def convert_to_mega(self, bytes):
        """Convert Bytes to Mega Bytes"""
        bytes = bytes // 1048576
        return bytes
