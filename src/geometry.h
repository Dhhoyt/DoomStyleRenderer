#pragma once

#include <stdbool.h>
#include "constants.h"

typedef struct {
    real_t x;
    real_t y;
} Point2;

typedef struct {
    real_t x;
    real_t y;
    real_t z;
} Point3;

typedef struct {
    Point2 start;
    Point2 end;
    int front_sector;
    int back_sector;
} Line;

Line new_point(real_t x1, real_t y1, real_t x2, real_t y2);

bool point_is_in_front_of(Line l, Point2 p);
bool point_is_behind(Line l, Point2 p);
bool point_is_on(Line l, Point2 p);

bool is_in_front_of(Line l1, Line l2);
bool is_behind(Line l1, Line l2);
bool is_on(Line l1, Line l2);

Point2 calculate_intersection(Line l1, Line l2);