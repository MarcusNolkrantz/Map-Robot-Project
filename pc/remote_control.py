# -*- coding: utf-8 -*-

"""
file: remote_control.py
author: juska933

Remote control module. Translates key presses to steering commands sent to the 
communication module.
"""


from pygame.constants import * 
import json

from interface import communication


MOTORS_ROTATE_LEFT		= 0
MOTORS_ROTATE_RIGHT		= 1
MOTORS_DRIVE_FORWARD	= 2
MOTORS_DRIVE_BACKWARD	= 3
MOTORS_DRIVE_LEFT		= 4
MOTORS_DRIVE_RIGHT		= 5
MOTORS_HALT				= 6

valid_keys = {K_LEFT, K_DOWN, K_UP, K_RIGHT}

# in order of "biggest" states
state_checks = [
    (MOTORS_DRIVE_LEFT, lambda keys_down: keys_down == {K_UP, K_LEFT}),
    (MOTORS_DRIVE_RIGHT, lambda keys_down: keys_down == {K_UP, K_RIGHT}),
    (MOTORS_DRIVE_FORWARD, lambda keys_down: keys_down == {K_UP}),
    (MOTORS_DRIVE_BACKWARD, lambda keys_down: keys_down == {K_DOWN}),
    (MOTORS_ROTATE_LEFT, lambda keys_down: keys_down == {K_LEFT}),
    (MOTORS_ROTATE_RIGHT, lambda keys_down: keys_down == {K_RIGHT}),
    (MOTORS_HALT, lambda keys_down: True),
]


class RemoteHandler:
    def __init__(self):
        self.keys_down = set()
        self.is_remote = True
        self.state = MOTORS_HALT

    def check_state(self):
        for state, check in state_checks:
            if check(self.keys_down):
                return state

    def maybe_update_state(self):
        state = self.check_state()
        if state != self.state:
            self.state = state 
            self.send_remote_command()

    def send_remote_command(self):
        data = {
            "id": "command",
            "type": int(self.state)
        }
        data = json.dumps(data)
        if self.is_remote:
            communication.send(data)

    def key_down(self, key):
        if key in valid_keys:
            self.keys_down.add(key)
        self.maybe_update_state()
    
    def key_up(self, key):
        if key in self.keys_down:
            self.keys_down.remove(key)
        self.maybe_update_state()
    