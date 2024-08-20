#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "rendering.h"
#include "geometry.h"

void reset_spans(ScreenBuffer* buf);
void render_recurse(Map* m, ScreenBuffer* buf, Point3 player_pos, real_t heading, real_t fov, int node_id);
void free_span(Span* s);

ScreenBuffer new_buffer() {
    ScreenBuffer buf;
    reset_spans(&buf);
    return buf;
}

void free_span(Span* s) {
    if(s == NULL) {
        return;
    }
    free_span(s->next);
    free(s);
}

void reset_spans(ScreenBuffer* buf) {
    for(int i = 0; i < WINDOW_WIDTH; i++) {
        buf->spans[i].start = -1;
        buf->spans[i].end = -1;
        free_span(buf->spans[i].next); //Might be redundant
    }
}

void render_map(Map* m, ScreenBuffer* buf, Point3 player_pos, real_t heading, real_t fov) {
    render_recurse(m, buf, player_pos, heading, fov, 0);
}

Pixel project_point(Point3 p, Point3 player_pos, real_t heading, real_t fov) {
    // Step 1: Translate the point relative to the player's position
    real_t dx = p.x - player_pos.x;
    real_t dy = p.y - player_pos.y;
    real_t dz = p.z - player_pos.z;

    // Step 2: Rotate the point around the z-axis based on the player's heading
    real_t cos_heading = cos(heading);
    real_t sin_heading = sin(heading);
    real_t cx = cos_heading * dx - sin_heading * dy;
    real_t cy = sin_heading * dx + cos_heading * dy;

    // Step 3: Project the 3D point to 2D screen space
    real_t distance_to_screen = WINDOW_WIDTH / (2.0 * tan(fov / 2.0));

    // Avoid division by zero, if the point is directly behind the player
    if (cx == 0) {
        Pixel res;
        res.x = -1;
        res.y = -1;
        return res;
    }
    Pixel res;
    res.x = (int)((cy / cx) * distance_to_screen + WINDOW_WIDTH / 2.0);
    res.y = (int)((dz / cx) * distance_to_screen + WINDOW_HEIGHT / 2.0);

    return res;
}

void render_recurse(Map* m, ScreenBuffer* buf, Point3 player_pos, real_t heading, real_t fov, int node_id) {
    Node* n = &m->tree.nodes[node_id];
    Point2 flat_player_pos;
    flat_player_pos.x = player_pos.x;
    flat_player_pos.y = player_pos.y;
    if(point_is_in_front_of(n->lines[0], flat_player_pos)) {
        if(n->front_child != 1) {
            render_recurse(m, buf, player_pos, heading, fov, n->front_child);
        }
        if(n->back_child != 1) {
            render_recurse(m, buf, player_pos, heading, fov, n->front_child);
        }
    } else {
        if(n->back_child != 1) {
            render_recurse(m, buf, player_pos, heading, fov, n->front_child);
        }
        if(n->front_child != 1) {
            render_recurse(m, buf, player_pos, heading, fov, n->front_child);
        }
    }
    
    for(int i = 0; i < n->line_count; i++) {
        
        Line l = n->lines[i];
        Sector sector;
        if(point_is_in_front_of(l, flat_player_pos)) {
            sector = m->sectors[l.front_sector];
        } else {
            sector = m->sectors[l.back_sector];
        }
        Point3 start;
        start.x = l.start.x;
        start.y = l.start.y;
        start.z = sector.floor_height;
        Pixel a = project_point(start, player_pos, heading, fov);
        start.z = sector.ceiling_height;
        Pixel b = project_point(start, player_pos, heading, fov);
        Point3 end;
        start.x = l.end.x;
        start.y = l.end.y;
        start.z = sector.floor_height;
        Pixel c = project_point(start, player_pos, heading, fov);
        start.z = sector.ceiling_height;
        Pixel d = project_point(start, player_pos, heading, fov);
        if((a.x > WINDOW_WIDTH && c.x > WINDOW_WIDTH) || (a.x > c.x) || (a.x < 0 && c.x < 0)) {
            continue;
        }
        ///TODO: Finish all checks
        if(a.x < c.x) {
            draw_wall(buf, a, b, c, d, sector.wall_texture);
        } else {
            draw_wall(buf, c, d, a, b, sector.wall_texture);
        }
    }
}

void draw_wall(ScreenBuffer* buf, Pixel bottom_left, Pixel top_left, Pixel bottom_right, Pixel top_right, int texture) {
    int dx = bottom_left.x - bottom_right.x;
    int dy_bot = bottom_left.y - bottom_right.y;
    int dy_top = top_left.y - top_right.y;
    real_t bot_slope = (real_t)dy_bot/dx;
    real_t top_slope = (real_t)dy_top/dx;
    int end;
    //Clamp bounds
    if(bottom_right.x + 1 > WINDOW_WIDTH) {
        end = WINDOW_WIDTH;
    } else {
        end = bottom_right.x + 1; //+1 because its an inclusive range
    }
    for(int i = bottom_left.x; i < end; i++) {
        int y_bot = (int)((real_t)(i - bottom_left.x)*bot_slope) + bottom_left.y;
        int y_top = (int)((real_t)(i - bottom_left.x)*top_slope) + top_left.y;
        Span s = buf->spans[i];
    }
}