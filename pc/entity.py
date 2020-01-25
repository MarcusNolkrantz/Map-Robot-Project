# -*- coding: utf-8 -*-

"""
Entity module.
file: entity.py
authors: juska933, osklu414

Entity module. The various entities drawn the client updates and draws.
"""


import os
import math
import time
import pygame

import const
from interface import communication


def load_sprite(img_name, size=None):
    """Load a sprite from file."""
    file_path = os.path.join(const.RESOURCES_PATH, img_name)
    surface = pygame.image.load(file_path)
    if size:
        surface = pygame.transform.scale(surface, size)
    return surface.convert_alpha()


class Entity:
    """Entity class."""

    def __init__(self, position, rotation=0, parent=None):
        """Initialize entity."""
        self.position = position
        self.rotation = rotation
        self.parent = parent
        if parent:
            parent.children.append(self)
        self.children = []

    def on_render(self, screen, screen_position, screen_rotation):
        """Custom render code here."""
        pass

    def on_loop(self):
        """Custom update code here."""
        pass

    def render(self, screen, parent_position=(0, 0), parent_rotation=0):
        """Render entity and its children, recursively."""
        position = (self.position[0] + parent_position[0], self.position[1] + parent_position[1])
        rotation = self.rotation + parent_rotation

        for child in self.children:
            child.render(screen, parent_position=position, parent_rotation=rotation)

        # call entity draw callback
        self.on_render(screen, position, rotation)

    def loop(self):
        """Loop code common for all entities."""
        self.on_loop()
        for child in self.children:
            child.loop()


class Robot(Entity):
    """Robot class. Has a DotCloud, side distance sensors and gyro."""

    SPRITE = None

    def __init__(self, map):
        """Initialize robot."""
        if not Robot.SPRITE:
            Robot.SPRITE = load_sprite("robot.png", size=const.ROBOT_SIZE)
        position = (const.TILE_WIDTH * const.ROBOT_ORIGIN[0], const.TILE_HEIGHT * const.ROBOT_ORIGIN[1])
        super().__init__(position, parent=map)
        self.dot_cloud = DotCloud(self)
        self.left_distance = 1000.0
        self.right_distance = 1000.0

        @communication.on_receive("robot")
        def on_robot(x, y, r):
            self.position = ((x + const.ROBOT_ORIGIN[0]) * const.TILE_SIZE[0], (y + const.ROBOT_ORIGIN[1]) * const.TILE_SIZE[1])
            self.rotation = r

        @communication.on_receive("sensor")
        def on_sensor(left, right, rot):
            self.left_distance = left
            self.right_distance = right

    def on_render(self, screen, screen_position, screen_rotation):
        """Draw robot."""
        # draw robot sprite TODO: rotate around center
        screen.blit(pygame.transform.rotate(Robot.SPRITE, screen_rotation), (screen_position[0] - Robot.SPRITE.get_width()/2, screen_position[1] - Robot.SPRITE.get_height()/2))
        # draw side distance sensors
        left_distance_start = (screen_position[0] + const.ROBOT_SIZE[0] // 2, screen_position[1] + const.ROBOT_SIZE[1] // 2)
        left_distance_end = (
            left_distance_start[0] + self.left_distance*(const.TILE_SIZE[0]/const.TILE_MM)*math.cos(-(screen_rotation+180)*math.pi/180),
            left_distance_start[1] + self.left_distance*(const.TILE_SIZE[0]/const.TILE_MM)*math.sin(-(screen_rotation+180)*math.pi/180)
        )
        pygame.draw.line(screen, (0, 0, 255), left_distance_start, left_distance_end, 1)

        right_distance_start = (screen_position[0] + const.ROBOT_SIZE[0] // 2, screen_position[1] + const.ROBOT_SIZE[1] // 2)
        right_distance_end = (
            right_distance_start[0] + self.right_distance*(const.TILE_SIZE[0]/const.TILE_MM)*math.cos(-screen_rotation*math.pi/180),
            right_distance_start[1] + self.right_distance*(const.TILE_SIZE[0]/const.TILE_MM)*math.sin(-screen_rotation*math.pi/180)
        )
        pygame.draw.line(screen, (0, 0, 255), right_distance_start, right_distance_end, 1)
    
    def on_loop(self):
        r = self.rotation
        #self.rotation = r + 1
        #self.position = (self.position[0] + 1, self.position[1])


class Dot(Entity):
    """Dot entity for RPlidar dotcloud."""
    
    COLOR = (255, 0, 0)

    def __init__(self, robot, angle, dist, quality):
        """Initialize dot entity."""
        origin_x, origin_y = robot.position
        origin_r = robot.rotation
        angle = (-angle - origin_r - 90)*(math.pi/180)
        x, y = origin_x + (dist*const.TILE_WIDTH/const.TILE_MM)*math.cos(angle), origin_y + (dist*const.TILE_HEIGHT/const.TILE_MM)*math.sin(angle)
        super().__init__((int(x), int(y)))
        self.quality = quality
        self.created = time.time()
    
    def on_render(self, screen, screen_position, screen_rotation):
        """Draw dot."""
        pygame.draw.circle(screen, Dot.COLOR, self.position, 1)
    

class DotCloud(Entity):
    """DotCloud entity for RPlidar dotcloud."""

    def __init__(self, robot):
        """Initialize dotcloud."""
        super().__init__((0, 0), parent=robot)
        self.dots = []
        self.dot_lifetime = 0.1
        self.robot = robot

        @communication.on_receive("rplidar")
        def on_rplidar(nodes):
            for node in nodes:
                dist = node["dist"]
                angle = node["angle"]
                quality = node["quality"]
                self.dots.append(Dot(self.robot, angle, dist, quality))
    
    def on_loop(self):
        """Remove old dots."""
        new_dots = []
        now = time.time()
        for dot in self.dots:
            lived = now - dot.created
            if lived <= self.dot_lifetime:
                new_dots.append(dot)
        self.dots = new_dots
    
    def on_render(self, screen, screen_position, screen_rotation):
        """Render the dots (they are not attached as children)."""
        for dot in self.dots:
            dot.on_render(screen, screen_position, screen_rotation)


class Tile(Entity):
    """Tile class."""

    SPRITES = []

    UNKNOWN = 0
    EMPTY = 1
    WALL = 2

    def __init__(self, map, col, row, tile_type=UNKNOWN):
        """Initialize tile."""
        if not Tile.SPRITES:
            Tile.SPRITES = [
                load_sprite("unknown_tile.png", size=const.TILE_SIZE),
                load_sprite("empty_tile.png", size=const.TILE_SIZE),
                load_sprite("wall_tile.png", size=const.TILE_SIZE)
            ]

        super().__init__((col*const.TILE_WIDTH, row*const.TILE_HEIGHT), parent=map)
        self.type = tile_type
        self.col = col
        self.row = row
    
    def set_type(self, tile_type):
        """Update tile type."""
        self.type = tile_type
    
    def on_render(self, screen, screen_position, screen_rotation):
        """Draw correct tile sprite."""
        screen.blit(Tile.SPRITES[self.type], screen_position)


class Map(Entity):
    """Map class. Has tiles and robot."""

    def __init__(self):
        """Initialize map."""
        super().__init__(const.MAP_POSITION)
        self.tiles = [[Tile(self, c, r) for c in range(const.MAP_COLS)] for r in range(const.MAP_ROWS)]
        self.robot = Robot(self)
        self.points = []
        
        @communication.on_receive("tile")
        def on_tile(col, row, type):
            self.set_tile(col, row, type)
        
        @communication.on_receive("map")
        def on_map(tiles):
            for r in range(const.MAP_ROWS):
                for c in range(const.MAP_COLS):
                    self.set_tile(c, r, tiles[r * const.MAP_COLS + c])
    
        @communication.on_receive("point")
        def on_point(col, row):
            if not col or not row:
                return
            self.points.append((col, row, time.time()))

    def set_tile(self, col, row, tile_type):
        """Update tile type."""
        self.tiles[row][col].set_type(tile_type)
    
    def on_render(self, screen, screen_position, screen_rotation):
        """Draw points."""
        new_points = []
        for p in self.points:
            pygame.draw.circle(screen, (0, 255, 0), (int(screen_position[0] + p[0]*const.TILE_SIZE[0]), int(screen_position[1] + p[1]*const.TILE_SIZE[1])), 1)
            if time.time() - p[2] < 0.1:
                new_points.append(p)
        self.points = new_points

class Interface(Entity):
    """Interface class. Contains buttons and text inputs."""

    def __init__(self):
        """Initialize interface."""
        super().__init__(const.INTERFACE_POSITION)

        x = const.INTERFACE_PADDING
        y = const.INTERFACE_PADDING
        w = const.INTERFACE_WIDTH - 2*const.INTERFACE_PADDING
        h = const.INTERFACE_FONT_SIZE

        self.steering_text = Text((x, y), self, text="steering data:")
        y += h
        self.steering_left_speed = Text((x, y), self)
        y += h
        self.steering_right_speed = Text((x, y), self)
        y += h
        self.steering_left_forward = Text((x, y), self)
        y += h
        self.steering_right_forward = Text((x, y), self)

        y += 2*h

        self.sensor_text = Text((x, y), self, text="sensor data:")
        y += h
        self.sensor_left = Text((x, y), self)
        y += h
        self.sensor_right = Text((x, y), self)
        y += h
        self.sensor_rot = Text((x, y), self)
        y += h
        self.robot_pos = Text((x, y), self)

        @communication.on_receive("sensor")
        def on_sensor(left, right, rot):
            self.sensor_left.set_text("left distance: {:.2f}".format(left))
            self.sensor_right.set_text("right distance: {:.2f}".format(right))
            self.sensor_rot.set_text("gyro rotation: {:.2f} deg".format(rot))

        @communication.on_receive("steering")
        def on_steering(left_speed, right_speed, left_forward, right_forward):
            self.steering_left_speed.set_text("left_speed: {:.2f}".format(left_speed))
            self.steering_right_speed.set_text("right_speed: {:.2f}".format(right_speed))
            self.steering_left_forward.set_text("left direction: {}".format("forward" if left_forward else "backward"))
            self.steering_right_forward.set_text("right direction: {}".format("forward" if right_forward else "backward"))

        @communication.on_receive("robot")
        def on_robot(x, y, r):
            self.robot_pos.set_text("robot position: ({:.2f}, {:.2f})".format(x, y))
        

class Text(Entity):
    """Text class. Renders text."""

    FONT = None

    def __init__(self, position, interface, text=""):
        """Initialize text."""
        if not Text.FONT:
            Text.FONT = pygame.font.SysFont(const.INTERFACE_FONT_NAME, const.INTERFACE_FONT_SIZE)
        super().__init__(position, parent=interface)
        self.text = text
    
    def on_render(self, screen, screen_position, screen_rotation):
        """Draw text."""
        text_surface = Text.FONT.render(self.text, False, const.INTERFACE_COLOR)
        screen.blit(text_surface, screen_position)

    def set_text(self, text):
        """Update text."""
        self.text = text
