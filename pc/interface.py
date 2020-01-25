# -*- coding: utf-8 -*-

"""
file: interface.py
authors: juska933, osklu414

Interface module. Used to communicate with the communication module over its
WiFi access point.
"""


import requests
import json
import socket

import const


# communication constants
MSG_END_HEADER = "__MSG_END__"


class Communication:
    """Communication module interface class."""

    def __init__(self):
        """Init communication module interface object."""
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.callbacks = {}
        self.last_read_buffer = ""

    def connect(self):
        """Connect socket."""
        try:
            self.s.connect((const.HOST, const.PORT))
        except:
            print("error when connecting to communication module")
    
    def close(self):
        """Close socket."""
        try:
            self.s.close()
        except:
            print("error when closing connection to communication module")

    def read(self):
        """Read socket message."""
        buf_size = 1024
        self.s.setblocking(False)
        msg = ""
        try:
            while True:
                cur = self.s.recv(buf_size)
                msg += cur.decode("utf-8")
                if len(cur) < buf_size:
                    return msg
        except:
            return ""
    
    def send(self, msg):
        """Send socket message."""
        try:
            msg = msg + MSG_END_HEADER
            self.s.send(msg.encode())
        except:
            print("error when sending message to communication module")
    
    def on_loop(self):
        """Read socket messages and handle packets."""
        data = self.last_read_buffer + self.read()
        if data:
            packets = data.split(MSG_END_HEADER)
            self.last_read_buffer = packets[-1]
            packets.pop()
            for packet in packets:
                self.handle_packet(packet) 
    
    def handle_packet(self, data):
        """Handle a single packet."""
        try:
            data = json.loads(data)
            if "id" not in data:
                return
            id = data.pop("id", None)
            if id and id in self.callbacks:
                for callback in self.callbacks[id]:
                    callback(**data)
            else:
                print("unhandled packet id " + data["id"])

        except:
            print("Error decoding json req {}".format(data))

    # RX

    def on_receive(self, id):
        """Register callback for messages from communication module."""
        def wrapper(fn):
            if not id in self.callbacks:
                self.callbacks[id] = []                
            self.callbacks[id].append(fn)

            return fn
        return wrapper


    # TX

    def transmit_command(self, command):
        """Send command."""
        data = {
            "id": "command",
            "type": command
        }
        data = json.dumps(data)
        self.send(data)
    
    def transmit_calibration(self, kp, kd):
        """Send calibration."""
        data = {
            "id": "calibration",
            "kp": float(kp),
            "kd": float(kd)
        }
        data = json.dumps(data)
        self.send(data)


# global communication object, can and should be imported to interface with
# the communication module
communication = Communication()