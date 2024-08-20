import pygame
import pygame_gui
import sys
import math
import struct

#https://lospec.com/palette-list/sweetie-16
background_color = pygame.Color("#1a1c2c")
grid_color = pygame.Color("#333c57")
highlight_color = pygame.Color("#94b0c2")
x_color = pygame.Color("#38b764")
y_color = pygame.Color("#41a6f6")
editing_color = pygame.Color("#29366f")
menu_color = pygame.Color("#1a1c2c")
border_color = pygame.Color("#566c86")

clock = pygame.time.Clock()

window_size = (320, 200)
position = (0,0)
scale = 50

click_pos = (0,0)
old_pos = (0,0)

padding = 30

menu_width = 160

class Polygon:
    def __init__(self):
        self.color = pygame.Color("#257179")
        self.floor_texture = 0 #byte
        self.ceil_texture = 0  #byte
        self.wall_texture = 0  #byte
        self.floor_height = 0  #float
        self.ceil_height = 0   #float
        self.points = []       #Array of 2d float tuples

polygons = []
constructed_polygon = []
edited_polygon = 0

bound = 100

pygame.init()
running = True

# Set up the display
screen = pygame.display.set_mode(window_size, pygame.RESIZABLE)
pygame.display.set_caption("Pawcalypse Editor")
manager = pygame_gui.UIManager((800, 600))

next_sector = pygame_gui.elements.UIButton(
    relative_rect=pygame.Rect((padding, padding), (100, 50)),
    text='Next Sector',
    manager=manager)

prev_sector = pygame_gui.elements.UIButton(
    relative_rect=pygame.Rect((padding, (2 * padding) + 50 ), (100, 50)),
    text='Prev Sector',
    manager=manager)

def clamp(low, x, high):
    return max(min(x, high), low)

def snap_to_grid(pos):
    x = round(pos[0] * 2)/2
    y = round(pos[1] * 2)/2
    #clamp teehee
    x = clamp(-bound, x, bound)
    y = clamp(-bound, y, bound)
    return (x, y)

def to_screen_coord(pos):
    x_top = position[0] - (window_size[0]/(scale * 2))
    y_top = position[1] - (window_size[1]/(scale * 2))
    x = (pos[0] - x_top) * scale
    y = (pos[1] - y_top) * scale
    return (x,y)

def from_screen_coord(pos):
    x_top = position[0] - (window_size[0]/(scale * 2))
    y_top = position[1] - (window_size[1]/(scale * 2))
    x = (pos[0]/scale) + x_top
    y = (pos[1]/scale) + y_top
    return (x,y)

def draw_grid(circle_size):
    global window_size
    x_top = position[0] - (window_size[0]/(scale * 2)) + menu_width/scale
    y_top = position[1] - (window_size[1]/(scale * 2))
    x_min = math.ceil(x_top - 1)
    y_min = math.ceil(y_top - 1)
    y = max(y_min, -bound - 1)
    draw_highlight = False
    highlight_coords = (0, 0)
    while(to_screen_coord((x_min,y))[1] < window_size[1] + circle_size):
        y += 1
        if y > bound:
            break
        if y != 0:
            pygame.draw.line(screen, grid_color, to_screen_coord((x_min,y)), to_screen_coord((x_min + window_size[0],y)))
        else:
            pygame.draw.line(screen, x_color, to_screen_coord((x_min,y)), to_screen_coord((x_min + window_size[0],y)))
    x = max(x_min, -bound - 1)
    while(to_screen_coord((x,y_min))[0] < window_size[0] + circle_size):
        x += 1
        if x > bound:
            break
        if x != 0:
            pygame.draw.line(screen, grid_color, to_screen_coord((x,y_min)), to_screen_coord((x,y + window_size[1])))
        else:
            pygame.draw.line(screen, y_color, to_screen_coord((x,y_min)), to_screen_coord((x,y + window_size[1])))
    y = max(y_min, -bound - 1)
    while(to_screen_coord((x_min,y))[1] < window_size[1] + circle_size):
        y += 1
        if y > bound:
            break
        x = max(x_min, -bound - 1)
        while(to_screen_coord((x,y))[0] < window_size[0] + circle_size):
            x += 1
            if x > bound:
                break
            screen_coords = to_screen_coord((x,y))
            pygame.draw.circle(screen, grid_color, screen_coords, circle_size)

def draw_polygons():
    line_width = int(scale/15)
    if len(constructed_polygon) >= 2:
        translated_points = []
        for i in constructed_polygon:
            translated_points.append(to_screen_coord(i)) 
        pygame.draw.lines(screen, editing_color, False, translated_points, line_width)
    for index, poly in enumerate(polygons):
        translated_points = []
        for i in poly.points:
            translated_points.append(to_screen_coord(i))
        line_width_2_electric_boogaloo = 0 if index == edited_polygon else line_width 
        pygame.draw.polygon(screen, poly.color, translated_points, line_width_2_electric_boogaloo)

def signed_area(points):
    n = len(points) - 1
    area = 0.0
    for i in range(n):
        x1, y1 = points[i]
        x2, y2 = points[i + 1]
        area += x1 * y2 - y1 * x2
    return area / 2.0

def is_convex_polygon(vertices):
    def cross_product(o, a, b):
        # Returns the cross product of vector OA and OB
        # A positive cross product indicates a counter-clockwise turn, negative indicates clockwise turn
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])
    
    n = len(vertices)
    if n < 3:
        return False  # A polygon must have at least 3 vertices
    
    # Check the direction of the first cross product
    direction = None
    for i in range(n):
        o, a, b = vertices[i], vertices[(i + 1) % n], vertices[(i + 2) % n]
        cross_prod = cross_product(o, a, b)
        if cross_prod != 0:
            direction = cross_prod > 0
            break
    
    if direction is None:
        return False  # All points are collinear, not a polygon
    
    # Check consistency of the cross product direction
    for i in range(n):
        o, a, b = vertices[i], vertices[(i + 1) % n], vertices[(i + 2) % n]
        cross_prod = cross_product(o, a, b)
        if cross_prod != 0:
            if (cross_prod > 0) != direction:
                return False
    
    return True

def handle_left_click(event):
    global edited_polygon
    for index, poly in enumerate(polygons):
        if is_point_in_convex_polygon(poly.points, from_screen_coord(event.pos)):
            edited_polygon = index
            return
    if not event.pos[0] < menu_width:
        handle_grid_click(event)

def handle_grid_click(event):
    global constructed_polygon
    click_pos_translated = from_screen_coord(event.pos)
    clicked_node = snap_to_grid(click_pos_translated)

    if len(constructed_polygon) > 1 and constructed_polygon[0] == clicked_node:
        if signed_area(constructed_polygon) != 0 and is_convex_polygon(constructed_polygon):
            new_polygon = Polygon()
            if signed_area(constructed_polygon) < 0:
                new_polygon.points = constructed_polygon
            else:
                new_polygon.points = constructed_polygon[::-1]
            polygons.append(new_polygon)
            constructed_polygon = []
        constructed_polygon = []                        
    else:
        if len(constructed_polygon) == 0 or clicked_node != constructed_polygon[-1]:
            constructed_polygon.append(clicked_node)

def draw_menu(dt):
    pygame.draw.rect(screen, menu_color, pygame.Rect(0, 0, menu_width, window_size[1]))
    pygame.draw.rect(screen, border_color, pygame.Rect(0, 0, menu_width, window_size[1]), 3)
    manager.update(dt/1000)
    manager.draw_ui(screen)
def is_point_inside_convex_polygon(polygon, point):
    def cross_product(o, a, b):
        # (a - o) x (b - o)
        return (a[0] - o[0]) * (b[1] - o[1]) - (a[1] - o[1]) * (b[0] - o[0])
    
    n = len(polygon)
    
    for i in range(n):
        o = polygon[i]
        a = polygon[(i + 1) % n]
        b = point
        cp = cross_product(o, a, b)
        
        if cp == 0:
            # Point is on the boundary
            return False
        elif cp < 0:
            # Point is outside
            return False
    
    # Point is inside
    return True

def is_point_in_convex_polygon(polygon, point):
    n = len(polygon)
    
    def cross_product(A, B):
        return A[0] * B[1] - A[1] * B[0]
    
    def vector(P, Q):
        return (Q[0] - P[0], Q[1] - P[1])

    # Calculate the initial cross product sign
    P1 = polygon[0]
    P2 = polygon[1]
    initial_vector = vector(P1, P2)
    point_vector = vector(P1, point)
    initial_cross_product = cross_product(initial_vector, point_vector)

    if initial_cross_product == 0:
        # Point is on the boundary of the first edge
        return False

    initial_sign = initial_cross_product > 0

    for i in range(1, n):
        P1 = polygon[i]
        P2 = polygon[(i + 1) % n]
        edge_vector = vector(P1, P2)
        point_vector = vector(P1, point)
        current_cross_product = cross_product(edge_vector, point_vector)

        if current_cross_product == 0:
            # Point is on the boundary of this edge
            return False

        current_sign = current_cross_product > 0

        if current_sign != initial_sign:
            return False

    return True

def save_polygons_to_file(polygons, filename):
    with open(filename, 'wb') as file:
        for polygon in polygons:
            # Save the basic attributes
            file.write(struct.pack('BBB', polygon.floor_texture, polygon.ceil_texture, polygon.wall_texture))
            file.write(struct.pack('ff', polygon.floor_height, polygon.ceil_height))
            
            # Save the number of points
            file.write(struct.pack('I', len(polygon.points)))
            
            # Save the points
            for point in polygon.points:
                file.write(struct.pack('ff', *point))

def read_polygons_from_file(filename):
    polygons = []
    with open(filename, 'rb') as file:
        while True:
            # Read the basic attributes
            data = file.read(3 + 2 * 4 + 4)
            if not data:
                break
            
            floor_texture, ceil_texture, wall_texture = struct.unpack('BBB', data[:3])
            floor_height, ceil_height = struct.unpack('ff', data[3:11])
            
            # Read the number of points
            num_points, = struct.unpack('I', data[11:15])
            
            # Read the points
            points = []
            for _ in range(num_points):
                point_data = file.read(2 * 4)
                point = struct.unpack('ff', point_data)
                points.append(point)
            
            polygon = Polygon()
            polygon.floor_texture = floor_texture
            polygon.ceil_texture = ceil_texture
            polygon.wall_texture = wall_texture
            polygon.floor_height = floor_height
            polygon.ceil_height = ceil_height
            polygon.points = points
            
            polygons.append(polygon)
    
    return polygons


def main():
    global window_size
    global running
    global scale
    global position
    global edited_polygon
    global constructed_polygon
    global polygons
    dragging = False
    while running:
        dt = clock.tick(144)
        mouse_pos = pygame.mouse.get_pos()
        mouse_pos_translated = from_screen_coord(mouse_pos)
        node_hovered = snap_to_grid(mouse_pos_translated)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_s:
                    save_polygons_to_file(polygons, "map.paw")
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if event.button == 1:
                    handle_left_click(event)

                elif event.button == 2:
                    dragging = True
                    old_pos = event.pos
                elif event.button == 3:
                    print("hkjsdf")
                    constructed_polygon = constructed_polygon[:-1]
            elif event.type == pygame.MOUSEBUTTONUP:
                if event.button == 2:
                    dragging = False
            elif event.type == pygame.MOUSEWHEEL:
                if event.y == 1:
                    scale += 1
                else:
                    scale -= 1
                scale = clamp(20, scale, 100)
            elif event.type == pygame.VIDEORESIZE:
                window_size = event.size
                print(window_size)
            elif event.type == pygame_gui.UI_BUTTON_PRESSED:
                if event.ui_element == next_sector and len(polygons) >= 1:
                    edited_polygon += 1
                    if edited_polygon >= len(polygons):
                        edited_polygon = 0
                elif event.ui_element == prev_sector and len(polygons) >= 1:
                    edited_polygon -= 1
                    if edited_polygon == -1:
                        edited_polygon = len(polygons) - 1
            manager.process_events(event)
        circle_size = scale/10
        
        if dragging:
            distance_moved_x = mouse_pos[0] - old_pos[0] 
            position_x = position[0] - distance_moved_x/scale
            distance_moved_y = mouse_pos[1] - old_pos[1]
            position_y = position[1] - distance_moved_y/scale
            position = (position_x, position_y)
            old_pos = mouse_pos 

        screen.fill(background_color)
        draw_grid(circle_size)
        if not dragging:
            pygame.draw.circle(screen, highlight_color, to_screen_coord(node_hovered), circle_size)
        draw_polygons()
        draw_menu(dt)
        pygame.display.flip()

    # Quit Pygame
    pygame.quit()
    sys.exit()

if __name__ == "__main__":
    main()