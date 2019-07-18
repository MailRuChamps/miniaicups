import random
import pyglet
from constants import WIDTH, X_CELLS_COUNT, Y_CELLS_COUNT


def show_coordinates(point):
    x, y = point
    pyglet.text.Label('{}, {}'.format(x, y), font_name='Times New Roman',
                      font_size=9,
                      color=(95, 99, 104, 255),
                      x=x, y=y,
                      anchor_x='center', anchor_y='center').draw()


def get_square_coordinates(point, width=WIDTH):
    x, y = point
    return (x - width, y - width,
            x + width, y - width,
            x + width, y + width,
            x - width, y + width)


def get_diagonals(point, width=WIDTH):
    x, y = point

    return [
        (x + width, y + width),
        (x - width, y + width),
        (x + width, y - width),
        (x - width, y - width)
    ]


def get_vert_and_horiz(point, width=WIDTH):
    x, y = point

    return [
        (x, y + width),
        (x - width, y),
        (x, y - width),
        (x + width, y),
    ]


def get_neighboring(point, width=WIDTH):
    return [
        *get_vert_and_horiz(point, width),
        *get_diagonals(point, width)
    ]


def get_territory_line(point, points):
    line_points = []

    p = point
    while p in points:
        line_points.append(p)
        x, y = p
        p = (x - WIDTH, y)
    start = (p[0] + WIDTH, p[1])

    p = point
    while p in points:
        line_points.append(p)
        x, y = p
        p = (x + WIDTH, y)
    end = (p[0] - WIDTH, p[1])

    return line_points, start, end


def get_line_coordinates(start, end, width=WIDTH):
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


def batch_draw_territory(points, color, redraw, width=WIDTH):
    if len(points) < 100:
        batch_draw(points, color, width)
        return

    if color not in TERRITORY_CACHE or redraw:
        lines = []
        excluded = set()
        for point in points:
            if point not in excluded:
                line_points, start, end = get_territory_line(point, points)
                excluded.update(line_points)
                coors = get_line_coordinates(start, end, width)
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


def batch_draw(points, color, width=WIDTH):
    width = round(width / 2)
    pyglet.gl.glColor4f(*[i/255 for i in color])
    pyglet.gl.glBegin(pyglet.gl.GL_QUADS)
    for point in points:
        square = get_square_coordinates(point, width)
        pyglet.graphics.glVertex2i(square[0], square[1])
        pyglet.graphics.glVertex2i(square[2], square[3])
        pyglet.graphics.glVertex2i(square[4], square[5])
        pyglet.graphics.glVertex2i(square[6], square[7])
    pyglet.gl.glEnd()


def draw_square(point, color, width=WIDTH):
    width = round(width / 2)
    coordinates = get_square_coordinates(point, width)
    pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, ('v2i', coordinates), ('c4B', 4 * color))


def draw_quadrilateral(coordinates, color):
    pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, ('v2i', coordinates), ('c4B', 4 * color))


def draw_line(point1, point2, color, width=WIDTH):
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
        img = pyglet.image.load(path)
        img.anchor_x = round(img.width / 2)
        img.anchor_y = round(img.height / 2)
        IMAGE_CACHE[path] = img

    return IMAGE_CACHE[path]


def draw_square_with_image(point, color, image_path, label=None, width=WIDTH):
    draw_square(point, color, width)

    x, y = point

    img = load_image(image_path)
    sprite = pyglet.sprite.Sprite(img=img, x=x, y=y)
    sprite.scale = 0.75 * (width / max(sprite.height, sprite.width))
    sprite.draw()

    if label is not None:
        pyglet.text.Label('{}'.format(label), font_name='Times New Roman',
                          font_size=round(WIDTH / 7),
                          color=(95, 99, 104, 255),
                          x=x + round(WIDTH / 2), y=y + round(WIDTH / 2),
                          anchor_x='right', anchor_y='top').draw()


def get_random_coordinates():
    x = random.randint(1, X_CELLS_COUNT) * WIDTH - round(WIDTH / 2)
    y = random.randint(1, Y_CELLS_COUNT) * WIDTH - round(WIDTH / 2)
    return x, y


def is_intersect(p1, p2, width=WIDTH):
    x1, y1 = p1
    x2, y2 = p2
    return abs(x1 - x2) < width and abs(y1 - y2) < width
