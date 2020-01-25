#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
file: client.py
authors: juska933, osklu414

Client module. PC program entry-point and main loop.
"""


import pygame

import const
import entity
import remote_control
from interface import communication

class Client:
    """Client class."""

    def __init__(self):
        """Initialize client."""
        pygame.init()
        self.screen = pygame.display.set_mode((const.SCREEN_WIDTH, const.SCREEN_HEIGHT), pygame.HWSURFACE | pygame.DOUBLEBUF)
        self.clock = pygame.time.Clock()
        self.entities = [
            entity.Map(),
            entity.Interface(),
        ]
        self.remote_handler = remote_control.RemoteHandler()
        self.running = True

        @communication.on_receive("message")
        def on_message(text):
            print("received message: ", text)
            pass

        communication.connect()
        while self.running:
            for event in pygame.event.get():
                self.on_event(event)
            self.on_loop()
            self.screen.fill(const.SCREEN_FILL)
            self.on_render()
            self.clock.tick(const.SCREEN_FRAMERATE)

        communication.close()
        pygame.quit()

    def on_event(self, event):
        """Handle events."""
        if event.type == pygame.QUIT:
            # quit when window is closed
            self.running = False
        elif event.type == pygame.KEYDOWN:
            self.remote_handler.key_down(event.key)
        elif event.type == pygame.KEYUP:
            self.remote_handler.key_up(event.key)

    def on_loop(self):
        """Update client."""
        # make sure any callbacks are called
        communication.on_loop()
        for e in self.entities:
            e.loop()

    def on_render(self):
        """Render client entities."""
        for e in self.entities:
            e.render(self.screen)
        pygame.display.flip()


if __name__ == "__main__":
    import traceback
    try:
        client = Client()
    except:
        traceback.print_exc()
        communication.close()
