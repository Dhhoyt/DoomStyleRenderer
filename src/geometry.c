#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h> 
#include "constants.h"
#include "geometry.h"

inline real_t height_projected_down(Line l, Point2 po) {
// Calculate the direction vector of the line
    real_t dir_x = l.end.x - l.start.x;
    real_t dir_y = l.end.y - l.start.y;

    // Calculate the vector from the point to the start of the line
    real_t vec_x = po.x - l.start.x;
    real_t vec_y = po.y - l.start.y;

    // Calculate the cross product of the direction vector and the point vector
    real_t cross_product = dir_x * vec_y - dir_y * vec_x;

    return cross_product; // Sign indicates the relative position
}

Line new_point(real_t x1, real_t y1, real_t x2, real_t y2) {
    Line l;
    l.start.x = x1;
    l.start.y = y1;
    l.end.x = x2;
    l.end.y = y2;
    return l;
}

//ASSUMES NOT PARALLEL
Point2 calculate_intersection(Line l1, Line l2) {
    Point2 intersection;
    real_t slope1 = (l1.start.y-l1.end.y) / (l1.start.x-l1.end.x);
    real_t slope2 = (l2.start.y-l2.end.y) / (l2.start.x-l2.end.x);

    // Handle vertical line cases
    if(FEQ(l1.start.x, l1.end.x)) {
        real_t distance = l1.start.x - l2.start.x;
        intersection.x = l1.start.x;
        intersection.y = (distance * slope2) + l2.start.y;
        return intersection;
    }
    if(FEQ(l2.start.x, l2.end.x)) {
        real_t distance = l2.start.x - l1.start.x;
        intersection.x = l2.start.x;
        intersection.y = (distance * slope1) + l1.start.y;
        return intersection;
    }

    // Calculate intersection for non-vertical lines
    real_t intercept1 = l1.start.y - (slope1 * l1.start.x);
    real_t intercept2 = l2.start.y - (slope2 * l2.start.x);
    
    if (FEQ(slope1, slope2)) {
        // Parallel lines case - no intersection or infinite intersections
        intersection.x = 0;
        intersection.y = 0;
        return intersection;
    }
    
    intersection.x = (intercept2 - intercept1) / (slope1 - slope2);
    intersection.y = (slope1 * intersection.x) + intercept1;

    return intersection;
}

bool point_is_in_front_of(Line l, Point2 po) {
    return height_projected_down(l, po) >= -EPSILON; 
}

bool is_in_front_of(Line l1, Line l2) {
    return point_is_in_front_of(l1, l2.start) && point_is_in_front_of(l1, l2.end);
}

bool point_is_behind(Line l, Point2 po) {
    return height_projected_down(l, po) <= EPSILON; 
}

bool is_behind(Line l1, Line l2) {
    return point_is_behind(l1, l2.start) && point_is_behind(l1, l2.end);
}

bool point_is_on(Line l, Point2 p) {  
    return fabs(height_projected_down(l, p)) <= EPSILON; 
}

bool is_on(Line l1, Line l2) {  
    return point_is_on(l1, l2.start) && point_is_on(l1, l2.end);
}