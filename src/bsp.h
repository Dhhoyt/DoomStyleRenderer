#pragma once

#include "geometry.h"
#include "constants.h"

typedef struct {
    real_t floor_height;
    real_t ceiling_height;
    int floor_texture;
    int ceiling_texture;
    int wall_texture;
    Point2* region;
    int vertex_count;
} Sector;

typedef struct {
    int sector; //Address of the sector definition in the map's list
    Point2* region;
    int point_count;
} Subsector;

typedef struct{
    Line* lines;
    int line_count;
    int size_allocated;
    int front_child;
    int back_child;
    int front_subsector;
    int back_subsector;
} Node;

typedef struct{
    Node* nodes; //nodes[0] is garunteed to be the root of the tree
    int node_count;
    int node_size_allocated;
    Subsector* subsectors;
    int subsector_count;
    int subsector_size_allocated;
} Tree;

typedef struct {
    Sector* sectors;
    int sector_count;
    Tree tree;
} Map;

Map new_map(Sector* sectors, int sector_count);
Tree new_tree(Line* lines, int line_count);