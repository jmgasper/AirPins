#!/usr/bin/env python3
"""Build AirPins' application icon: a vector version of the owner's artwork
(resources/branding/AirPins.png) -- an isometric green board with a black
40-pin style header, gold pins and four jumper wires (red, yellow, blue,
green) arching off the back row. Readable from 16 px up: small sizes drop
the board's details.

    python3 tools/make-icon.py resources/airpins-icon.hvif [preview.png]

Needs Pillow for the preview only.
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import hvif  # noqa: E402

C = hvif.hex_color

# isometric axes on the icon canvas, per board unit
U = (0.80, 0.50)     # along the header, to the lower right
V = (0.72, -0.38)    # across the header, away from the viewer
W = (0.0, -1.0)      # up

SCALE = 1.0
OFFSET = (0.0, 0.0)


def project(u, v, w=0.0):
    x = u * U[0] + v * V[0] + w * W[0]
    y = u * U[1] + v * V[1] + w * W[1]
    return (OFFSET[0] + x * SCALE, OFFSET[1] + y * SCALE)


def poly(points3d):
    return {'closed': True, 'points': [project(*p) for p in points3d]}


def rounded_quad(u0, u1, v0, v1, w, radius, steps=4):
    """A rectangle on the board's plane with rounded corners."""
    points = []
    corners = [(u1 - radius, v0 + radius, -90), (u1 - radius, v1 - radius, 0),
               (u0 + radius, v1 - radius, 90), (u0 + radius, v0 + radius, 180)]
    for cu, cv, start in corners:
        for i in range(steps + 1):
            a = math.radians(start + 90.0 * i / steps)
            points.append((cu + radius * math.cos(a), cv + radius * math.sin(a), w))
    return points


def box_faces(u0, u1, v0, v1, w0, w1):
    """The three visible faces of a box: front (v = v0), right (u = u1), top."""
    front = [(u0, v0, w0), (u1, v0, w0), (u1, v0, w1), (u0, v0, w1)]
    right = [(u1, v0, w0), (u1, v1, w0), (u1, v1, w1), (u1, v0, w1)]
    top = [(u0, v0, w1), (u1, v0, w1), (u1, v1, w1), (u0, v1, w1)]
    return front, right, top


def wire_path(u, v, w):
    """A jumper wire leaving a housing's top, arching up and over to the
    right and ending lower, beyond the header."""
    start = (u, v, w)
    end = (u + 8.0, v + 7.0, w + 1.0)
    c1 = (u, v, w + 19.0)
    c2 = (end[0] + 1.5, end[1] + 1.5, end[2] + 17.0)
    p0, p1, p2, p3 = (project(*p) for p in (start, c1, c2, end))
    # HVIF point: (point, control in, control out)
    return {'closed': False, 'points': [(p0, p0, p1), (p3, p2, p3)]}


# ------------------------------------------------------------------ layout
PIN_COLUMNS = 8
PITCH = 3.7
HEADER = (2.0, 2.0 + PIN_COLUMNS * PITCH, 0.0, 8.0, 0.0, 5.5)   # u0 u1 v0 v1 w0 w1
ROWS = (2.0, 6.0)
PIN_TOP = 13.0
PIN_HALF = 0.85
WIRE_COLUMNS = (1, 3, 5, 7)
HOUSING_TOP = 14.0
BOARD = (-6.0, 35.0, -21.0, 12.0)


def layout_points():
    """Every point that must fit on the canvas."""
    pts = []
    u0, u1, v0, v1 = BOARD
    for p in rounded_quad(u0, u1, v0, v1, 0, 4):
        pts.append((p[0], p[1], p[2]))
        pts.append((p[0], p[1], -2.2))
    for col in WIRE_COLUMNS:
        u = HEADER[0] + PITCH * (col + 0.5)
        v = ROWS[1]
        path = wire_path(u, v, HOUSING_TOP)
        # the curve's extreme stays within its control polygon's hull, so the
        # top of the arch is about three quarters of the way to the controls
        pts.append((u, v, HOUSING_TOP + 14.5))
        pts.append((u + 4.0, v + 3.5, HOUSING_TOP + 14.5))
        pts.append((u + 9.5, v + 8.5, HOUSING_TOP + 1.0))
    return pts


def fit():
    global SCALE, OFFSET
    SCALE, OFFSET = 1.0, (0.0, 0.0)
    xs, ys = zip(*(project(*p) for p in layout_points()))
    margin = 2.5
    SCALE = min((64 - 2 * margin) / (max(xs) - min(xs)),
                (64 - 2 * margin) / (max(ys) - min(ys)))
    width = (max(xs) - min(xs)) * SCALE
    height = (max(ys) - min(ys)) * SCALE
    OFFSET = (margin - min(xs) * SCALE + (64 - 2 * margin - width) / 2,
              margin - min(ys) * SCALE + (64 - 2 * margin - height) / 2)


# ------------------------------------------------------------------ styles
def lin(a, b, stops):
    return hvif.linear_gradient(a, b, stops)


def build():
    fit()
    styles, paths, shapes = [], [], []

    def style(s):
        styles.append(s)
        return len(styles) - 1

    def path(p):
        paths.append(p)
        return len(paths) - 1

    def shape(style_index, path_indices, **extra):
        entry = {'style': style_index, 'paths': list(path_indices)}
        entry.update(extra)
        shapes.append(entry)

    LARGE = {'lod': (0.5, 4.0)}
    SMALL = {'lod': (0.0, 0.5)}
    DETAIL = {'lod': (0.75, 4.0)}

    u0, u1, v0, v1 = BOARD
    top = rounded_quad(u0, u1, v0, v1, 0.0, 4)
    bottom = [(p[0], p[1], -2.2) for p in top]

    # soft shadow under the board
    shadow_pts = [(p[0] + 1.5, p[1] - 1.5, -3.2) for p in top]
    s_shadow = style({'color': (0, 0, 0, 60)})
    shape(s_shadow, [path(poly(shadow_pts))], **LARGE)

    # board edge (gold): the outline of top and bottom together
    edge_style = style(lin(project(u0, v0, 0), project(u0, v0, -2.2), [
        (0.0, C('e8c55a')), (1.0, C('9c7a18'))]))
    edge_pts = top + bottom
    # convex outline of top+bottom: the board is convex, a hull does it
    hull = convex_hull([project(*p) for p in edge_pts])
    shape(edge_style, [path({'closed': True, 'points': hull})])

    board_style = style(lin(project(u0, v1, 0), project(u1, v0, 0), [
        (0.0, C('3fb35b')), (0.55, C('23913f')), (1.0, C('17732f'))]))
    board_path = path(poly(top))
    shape(board_style, [board_path])
    shape(style({'color': C('0e4d1f', 120)}), [board_path],
          transformers=[{'type': 'contour', 'width': -0.6, 'join': 1, 'miter': 4}], **DETAIL)

    # traces on the board (lighter green lines), large sizes only
    trace = style({'color': C('6fd08a', 150)})
    trace_paths = []
    for dv, w_extra in ((-6.0, 0), (-9.0, 0)):
        trace_paths.append(path({'closed': False, 'points': [
            project(u0 + 6, v0 + 4.5 - dv * 0.0 + (dv + 15) * 0.6, 0),
            project(u0 + 12, v0 + 4.5 + (dv + 15) * 0.6, 0),
            project(u0 + 16, v0 + 1.0 + (dv + 15) * 0.6, 0),
            project(u1 - 4, v0 + 1.0 + (dv + 15) * 0.6, 0)]}))
    shape(trace, trace_paths, transformers=[{'type': 'stroke', 'width': 0.7, 'join': 1, 'cap': 1, 'miter': 4}], **DETAIL)

    # mounting hole: gold ring with a dark centre
    hu, hv = u0 + 25.0, v0 + 5.0
    ring = path(poly(ellipse_points(hu, hv, 3.0)))
    hole = path(poly(ellipse_points(hu, hv, 1.6)))
    shape(style(lin(project(hu - 3, hv, 0), project(hu + 3, hv, 0), [
        (0.0, C('f3d77a')), (1.0, C('b8901f'))])), [ring], **LARGE)
    shape(style({'color': C('0b2e14')}), [hole], **LARGE)

    # raspberry badge: a white berry of small circles and two leaves
    ru, rv = u0 + 9.0, v0 + 12.0
    berry = []
    for du, dv in ((0, 0), (2.2, 0.0), (-2.2, 0.0), (1.1, 1.9), (-1.1, 1.9),
                   (1.1, -1.9), (-1.1, -1.9), (0.0, 3.8)):
        berry.append(path(poly(ellipse_points(ru + du, rv - dv, 1.15, steps=10))))
    leaf_style = style({'color': C('ffffff', 230)})
    shape(leaf_style, berry, **DETAIL)
    leaves = [path(poly([(ru - 4.0, rv + 4.6, 0), (ru - 0.3, rv + 3.6, 0), (ru - 2.2, rv + 2.2, 0)])),
              path(poly([(ru + 4.0, rv + 4.6, 0), (ru + 0.3, rv + 3.6, 0), (ru + 2.2, rv + 2.2, 0)]))]
    shape(leaf_style, leaves, **DETAIL)

    # header block
    hu0, hu1, hv0, hv1, hw0, hw1 = HEADER
    front, right, topface = box_faces(hu0, hu1, hv0, hv1, hw0, hw1)
    shape(style({'color': C('141414')}), [path(poly(front))])
    shape(style({'color': C('202020')}), [path(poly(right))])
    shape(style(lin(project(hu0, hv0, hw1), project(hu1, hv1, hw1), [
        (0.0, C('3c3c3c')), (1.0, C('2a2a2a'))])), [path(poly(topface))])
    # the block is several plastic sections: grooves between the columns
    grooves = []
    for col in range(1, PIN_COLUMNS):
        u = hu0 + PITCH * col
        grooves.append(path({'closed': False, 'points': [
            project(u, hv0, hw0 + 0.4), project(u, hv0, hw1),
            project(u, hv1, hw1)]}))
    shape(style({'color': C('000000', 200)}), grooves,
          transformers=[{'type': 'stroke', 'width': 0.45, 'join': 0, 'cap': 0, 'miter': 4}], **LARGE)

    gold = style(lin((0, 0), (0, 0), [(0.0, C('fff0a8')), (0.45, C('e9be3c')), (1.0, C('a8800f'))]))
    gold_side = style({'color': C('9a7410')})
    cap = style({'color': C('fff6c8')})

    def pin(u, v, top_w=PIN_TOP):
        f, r, t = box_faces(u - PIN_HALF, u + PIN_HALF, v - PIN_HALF, v + PIN_HALF, hw1, top_w)
        shape(gold_side, [path(poly(r))])
        a, b = project(u - PIN_HALF, v - PIN_HALF, top_w), project(u + PIN_HALF, v - PIN_HALF, top_w)
        styles[gold] = lin(a, b, [(0.0, C('fff0a8')), (0.5, C('e9be3c')), (1.0, C('a8800f'))])
        shape(gold, [path(poly(f))])
        shape(cap, [path(poly(t))], **LARGE)

    def housing(u, v):
        h = 1.6
        f, r, t = box_faces(u - h, u + h, v - h, v + h, hw1, HOUSING_TOP)
        shape(style({'color': C('101010')}), [path(poly(f))])
        shape(style({'color': C('1c1c1c')}), [path(poly(r))])
        shape(style({'color': C('2e2e2e')}), [path(poly(t))])

    wire_colors = (('e52421', '8f0f0d'), ('f6c90e', '9a7a00'),
                   ('2266e8', '0f3b8c'), ('29b34a', '136b27'))
    wire_cols = dict(zip(WIRE_COLUMNS, wire_colors))

    # back row first: pins, or housings with wires
    for col in range(PIN_COLUMNS):
        u = hu0 + PITCH * (col + 0.5)
        if col in wire_cols:
            housing(u, ROWS[1])
        else:
            pin(u, ROWS[1])
    for col in sorted(wire_cols, reverse=True):
        u = hu0 + PITCH * (col + 0.5)
        light, dark = wire_cols[col]
        wp = path(wire_path(u, ROWS[1], HOUSING_TOP - 0.3))
        dark_style, light_style = style({'color': C(dark)}), style({'color': C(light)})
        shape(dark_style, [wp],
              transformers=[{'type': 'stroke', 'width': 3.4 * SCALE, 'join': 1, 'cap': 0, 'miter': 4}], **LARGE)
        shape(light_style, [wp],
              transformers=[{'type': 'stroke', 'width': 2.4 * SCALE, 'join': 1, 'cap': 0, 'miter': 4}], **LARGE)
        # below 32 pixels: thicker, without the dark rim
        shape(light_style, [wp],
              transformers=[{'type': 'stroke', 'width': 3.6 * SCALE, 'join': 1, 'cap': 0, 'miter': 4}], **SMALL)
        shape(style({'color': (255, 255, 255, 110)}), [wp],
              transformers=[{'type': 'stroke', 'width': 0.6 * SCALE, 'join': 1, 'cap': 0, 'miter': 4}], **DETAIL)
    # front row
    for col in range(PIN_COLUMNS):
        pin(hu0 + PITCH * (col + 0.5), ROWS[0])

    return {'styles': styles, 'paths': paths, 'shapes': shapes}


def ellipse_points(u, v, r, steps=12):
    return [(u + r * math.cos(2 * math.pi * i / steps),
             v + r * math.sin(2 * math.pi * i / steps), 0.0) for i in range(steps)]


def convex_hull(points):
    pts = sorted(set((round(x, 3), round(y, 3)) for x, y in points))
    if len(pts) < 3:
        return pts

    def cross(o, a, b):
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])
    lower, upper = [], []
    for p in pts:
        while len(lower) >= 2 and cross(lower[-2], lower[-1], p) <= 0:
            lower.pop()
        lower.append(p)
    for p in reversed(pts):
        while len(upper) >= 2 and cross(upper[-2], upper[-1], p) <= 0:
            upper.pop()
        upper.append(p)
    return lower[:-1] + upper[:-1]


if __name__ == '__main__':
    out = sys.argv[1] if len(sys.argv) > 1 else 'airpins-icon.hvif'
    icon = build()
    data = hvif.encode(icon)
    hvif.decode(data)
    with open(out, 'wb') as f:
        f.write(data)
    print('%s: %d bytes, %d shapes, %d paths' % (out, len(data), len(icon['shapes']), len(icon['paths'])))
    svg = out.rsplit('.', 1)[0] + '.svg'
    with open(svg, 'w') as f:
        f.write(hvif.to_svg(icon))
    if len(sys.argv) > 2:
        sizes = [16, 32, 64, 256]
        from PIL import Image
        sheet = Image.new('RGBA', (sum(sizes) + 10 * len(sizes), 256), (216, 216, 216, 255))
        x = 0
        for size in sizes:
            # what Haiku's renderer shows at this size: the levels of detail
            scale = size / 64.0
            visible = dict(icon)
            def shown(shape):
                low, high = shape.get('lod', (0.0, 4.0))
                return low <= scale and (scale < high or high >= 4.0)
            visible['shapes'] = [shape for shape in icon['shapes'] if shown(shape)]
            image = hvif.preview(visible, size)
            sheet.paste(image, (x, 256 - size), image)
            x += size + 10
        sheet.save(sys.argv[2])
        print('%s: preview' % sys.argv[2])
