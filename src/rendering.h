#pragma once

#include "bsp.h"
#include "constants.h"

typedef struct Span {
    int start;
    int end;
    struct Span* next;
} Span;

typedef struct {
    Span spans[WINDOW_WIDTH];
    char colors[WINDOW_HEIGHT * WINDOW_WIDTH * 3];
} ScreenBuffer;

typedef struct {
    int x;
    int y;
} Pixel;

ScreenBuffer new_buffer();
Pixel project_point(Point3 p, Point3 player_pos, real_t heading, real_t fov);
void render_map(Map* m, ScreenBuffer* buf, Point3 player_pos, real_t heading, real_t fov);