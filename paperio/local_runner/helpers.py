import os
import random
import pyglet
from constants import CONSTS


def show_coordinates(point):
    x, y = point
    pyglet.text.Label('{}, {}'.format(x, y), font_name='Times New Roman',
                      font_size=9,
                      color=(95, 99, 104, 255),
                      x=x, y=y,
                      anchor_x='center', anchor_y='center').draw()


def get_square_coordinates(point):
    width = round(CONSTS.WIDTH / 2)
    x, y = point
    return (x - width, y - width,
            x + width, y - width,
            x + width, y + width,
            x - width, y + width)


def get_diagonals(point):
    width = CONSTS.WIDTH
    x, y = point

    return [
        (x + width, y + width),
        (x - width, y + width),
        (x + width, y - width),
        (x - width, y - width)
    ]


def get_vert_and_horiz(point):
    width = CONSTS.WIDTH
    x, y = point

    return [
        (x, y + width),
        (x - width, y),
        (x, y - width),
        (x + width, y),
    ]


def get_neighboring(point):
    return [
        *get_vert_and_horiz(point),
        *get_diagonals(point)
    ]


def get_territory_line(point, points):
    line_points = []

    p = point
    while p in points:
        line_points.append(p)
        x, y = p
        p = (x - CONSTS.WIDTH, y)
    start = (p[0] + CONSTS.WIDTH, p[1])

    p = point
    while p in points:
        line_points.append(p)
        x, y = p
        p = (x + CONSTS.WIDTH, y)
    end = (p[0] - CONSTS.WIDTH, p[1])

    return line_points, start, end


def get_line_coordinates(start, end):
    width = CONSTS.WIDTH
    width = round(width / 2)
    x1, y1 = start
    x2, y2 = end
    return [
        (x2 + width, y2 + width),
        (x2 + width, y2 - width),
        (x1 - width, y1 - width),
        (x1 - width, y1 + width),
    ]


TERRITORY_CACHE = {}


def batch_draw_territory(points, color, redraw):
    if len(points) < 100:
        batch_draw(points, color)
        return

    if color not in TERRITORY_CACHE or redraw:
        lines = []
        excluded = set()
        for point in points:
            if point not in excluded:
                line_points, start, end = get_territory_line(point, points)
                excluded.update(line_points)
                coors = get_line_coordinates(start, end)
                lines.append(coors)
        TERRITORY_CACHE[color] = [len(points), lines]
    else:
        lines = TERRITORY_CACHE[color][1]

    pyglet.gl.glColor4f(*[i/255 for i in color])
    pyglet.gl.glBegin(pyglet.gl.GL_QUADS)
    for line in lines:
        for coor in line:
            pyglet.graphics.glVertex2i(*coor)
    pyglet.gl.glEnd()


def batch_draw(points, color):
    pyglet.gl.glColor4f(*[i/255 for i in color])
    pyglet.gl.glBegin(pyglet.gl.GL_QUADS)
    for point in points:
        square = get_square_coordinates(point)
        pyglet.graphics.glVertex2i(square[0], square[1])
        pyglet.graphics.glVertex2i(square[2], square[3])
        pyglet.graphics.glVertex2i(square[4], square[5])
        pyglet.graphics.glVertex2i(square[6], square[7])
    pyglet.gl.glEnd()


def draw_square(point, color):
    coordinates = get_square_coordinates(point)
    pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, ('v2i', coordinates), ('c4B', 4 * color))


def draw_quadrilateral(coordinates, color):
    pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, ('v2i', coordinates), ('c4B', 4 * color))


def draw_line(point1, point2, color, width=None):
    if not width:
        width = CONSTS.WIDTH
    x1, y1 = point1
    x2, y2 = point2

    width = round(width / 2)

    coordinates = (x1 - width, y1,
                   x1 + width, y1,
                   x2 + width, y2,
                   x2 - width, y2)

    if y1 == y2:
        coordinates = (x1, y1 + width,
                       x1, y1 - width,
                       x2, y2 - width,
                       x2, y2 + width)

    pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, ('v2i', coordinates), ('c4B', 4 * color))


def in_polygon(x, y, xp, yp):
    c = 0
    for i in range(len(xp)):
        if (((yp[i] <= y and y < yp[i - 1]) or (yp[i - 1] <= y and y < yp[i])) and \
                (x > (xp[i - 1] - xp[i]) * (y - yp[i]) / (yp[i - 1] - yp[i]) + xp[i])): c = 1 - c
    return c


IMAGE_CACHE = {}


def load_image(path):
    if path not in IMAGE_CACHE:
        base_dir = os.path.dirname(os.path.realpath(__file__))
        absolute_path = os.path.join(base_dir, path)
        img = pyglet.image.load(absolute_path)
        img.anchor_x = round(img.width / 2)
        img.anchor_y = round(img.height / 2)
        IMAGE_CACHE[path] = img

    return IMAGE_CACHE[path]


def draw_square_with_image(point, color, image_path):
    width = CONSTS.WIDTH
    draw_square(point, color)

    x, y = point

    img = load_image(image_path)
    sprite = pyglet.sprite.Sprite(img=img, x=x, y=y)
    sprite.scale = 0.75 * (width / max(sprite.height, sprite.width))
    sprite.draw()


def get_random_coordinates():
    x = random.randint(1, CONSTS.X_CELLS_COUNT) * CONSTS.WIDTH - round(CONSTS.WIDTH / 2)
    y = random.randint(1, CONSTS.Y_CELLS_COUNT) * CONSTS.WIDTH - round(CONSTS.WIDTH / 2)
    return x, y


def is_intersect(p1, p2):
    width = CONSTS.WIDTH
    x1, y1 = p1
    x2, y2 = p2
    return abs(x1 - x2) < width and abs(y1 - y2) < width
