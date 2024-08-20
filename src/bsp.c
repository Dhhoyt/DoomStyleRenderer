#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include "constants.h"
#include "bsp.h"
#include "geometry.h"

Node new_node();

void partition(Tree* t, int node_id, Line* lines, int line_count);
int push_node(Tree* t, Node n);
int push_line(Line** list, Line l, int* allocated_size, int* line_count);
int push_subsector(Tree* t, Subsector s);
void destroy_tree(Tree* t);
void destroy_node_and_children(Tree* t, int node_id);
void destroy_subsectors(Tree* t);
Line rearrange_lines(Line** line_list, int* allocated_size, int* line_count, int overlap_index, Line to_add);

Point2* remove_colinear(Point2* input, int input_length, int* output_point_count);
Point2 center(Point2* poly, int vertex_count);

void sort_and_push(Point2 a, Point2 b, Point2* segments, int* total_points) {
    if(FEQ(a.x,b.x)) {
        segments[*total_points] = a;
        *total_points += 1;
    } else if (a.x < b.x) {
        segments[*total_points] = a;
        segments[*total_points + 1] = b;
        *total_points += 2;
    } else {
        segments[*total_points] = b;
        segments[*total_points + 1] = a;
        *total_points += 2;
    }
}
//"Ownership" of sectors gets passed here. It is the job of Map to free it now.
Map new_map(Sector* sectors, int sector_count) {
    Map m;
    m.sectors = sectors;
    m.sector_count = sector_count;
    Line* lines = malloc(sizeof(Line) * INIT_LIST_SIZE);
    int allocated_size = INIT_LIST_SIZE;
    int line_count = 0;
    for(int i = 0; i < sector_count; i++) {
        Sector s = sectors[i];
        Point2 poly_center = center(s.region, s.vertex_count);
        for(int j = 0; j < s.vertex_count; j++){
            Line l;
            Point2 a = s.region[j];
            Point2 b = s.region[(j + 1) % s.vertex_count];
            if(FEQ(a.x, b.x)) {
                if(a.y < b.y) {
                    l.start = a;
                    l.end = b;
                } else {
                    l.start = b;
                    l.end = a;
                }
            } else if (a.x < b.x){
                l.start = a;
                l.end = b;
            } else {
                l.start = b;
                l.end = a;
            }
            if(point_is_in_front_of(l, poly_center)) {
                l.front_sector = i;
                l.back_sector = -1;
            } else {
                l.front_sector = -1;
                l.back_sector = i;
            }
            push_line(&lines, l, &allocated_size, &line_count);
        }
    }
    /* 
     * go through all the lines in the list
     * if there is overlap, sort the points, (which you can do via an insertion sort thing, just sort the start and end seperately) then
     * you have 2-4 points, 1-3 lines
     * 1: replace the first line, remove the second and shift everything higher down one
     * 2: replace the two lines
     * 3: replace the two lines with two then append to the list
     * repeat
     */
    for(int i = 0; i < line_count; i++) {
       for(int j = i + 1; j < line_count; j++) {
            Line a = lines[i];
            Line b = lines[j];
            if (!is_on(a, b)) {
                continue;
            }
            if(a.start.x >= b.end.x || b.start.x >= a.end.x) {
                continue;
            }
            Point2 segments[4];
            int total_points = 0;
            sort_and_push(a.start,b.start,segments,&total_points);
            sort_and_push(a.end,b.end,segments,&total_points);
            
            //Modify list
            Line l;
            if (a.front_sector == -1) {
                l.front_sector = b.front_sector;
                l.back_sector = a.back_sector;
            } else {
                l.front_sector = a.front_sector;
                l.back_sector = b.back_sector;
            }
            l.start = segments[0];
            l.end = segments[1];
            switch (total_points)
            {
            case 2:
                lines[i] = l;
                for(int k = j; k < line_count - 1; k++) {
                    lines[k] = lines[k+1];
                }
                line_count -= 1;
                j--;
                break;
            case 3:
                lines[i] = l;
                l.start = segments[1];
                l.end = segments[2];
                lines[j] = l;
                break;
            case 4:
                lines[i] = l;
                l.start = segments[1];
                l.end = segments[2];
                lines[j] = l;
                l.start = segments[2];
                l.end = segments[3];
                push_line(&lines, l, &allocated_size, &line_count);
                break;
            
            default:
                printf("This should never be hit. Something has gone horribly wrong");
                exit(EXIT_FAILURE);
            }
        } 
    }
    m.tree = new_tree(lines, line_count);
    free(lines);
    return m;
}

Point2 center(Point2* poly, int vertex_count) {
    //Average of points. Garunteed to be inside a convex polygon
    real_t x = poly[0].x;
    real_t y = poly[0].y;
    for(int i = 1; i < vertex_count; i++) {
        x += poly[i].x;
        y += poly[i].y;
    }
    Point2 p;
    p.x = x/(real_t)vertex_count;
    p.y = y/(real_t)vertex_count;
    return p;
}

Point2* remove_colinear(Point2* poly, int vertex_count, int* output_point_count) {
    Point2* res = malloc(sizeof(Point2) * vertex_count);
    *output_point_count = 0;
    for(int i = 0; i < vertex_count; i++) {
        Point2 prev = poly[(i - 1 + vertex_count) % vertex_count];
        Point2 curr = poly[i];
        Point2 next = poly[(i + 1) % vertex_count];
        //Check for infinite slope
        if(FEQ(curr.x, next.x) || FEQ(curr.x, prev.x)) {
            if(FEQ(curr.x, next.x) && FEQ(curr.x, prev.x)) {
                res[*output_point_count] = curr;
                *output_point_count += 1;
            }
            continue;
        } 
        double slope_1 = (prev.y - curr.y) / (prev.x - curr.x);
        double slope_2 = (curr.x - curr.y) / (curr.x - curr.x);
        if(slope_1 != slope_2) {
            res[*output_point_count] = curr;
            *output_point_count += 1;
        }
    }
    return res;
}

Node new_node() {
    Node n = {0};
    n.lines = malloc(sizeof(Line) * INIT_LIST_SIZE);
    n.size_allocated = INIT_LIST_SIZE;
    n.front_child = -1;
    n.back_child = -1;
    n.front_subsector = -1;
    n.back_subsector = -1;
    return n;
}

Tree new_tree(Line* lines, int line_count) {
    Tree t = {0};
    t.nodes = malloc(sizeof(Node) * INIT_LIST_SIZE);
    t.node_count = 0;
    t.node_size_allocated = INIT_LIST_SIZE;
    t.subsectors = malloc(sizeof(Subsector) * INIT_LIST_SIZE);
    t.subsector_count = 0;
    t.subsector_size_allocated = INIT_LIST_SIZE;
    Node root_node = new_node();
    int root_id = push_node(&t, root_node);
    partition(&t, root_id, lines, line_count);
    return t;
}

void recurse_one_side(int count, int node_id, Tree* t, bool left, Line* list, int line_count, int new_sector) {
    if(count > 0) {
        Node child = new_node();
        int new_child_id = push_node(t, child);
        Node* n = &t->nodes[node_id];
        if(left) {
            n->front_child = new_child_id;
        } else {
            n->back_child = new_child_id;
        }
        partition(t, new_child_id, list, line_count);
    } else {
        Subsector s;
        s.sector = new_sector;
        if(left) {
            t->nodes[node_id].front_subsector = push_subsector(t, s);
        } else {
            t->nodes[node_id].back_subsector = push_subsector(t, s);
        }
    }
    free(list);
}

void partition(Tree* t, int node_id, Line* lines, int line_count) {
    Node* n = &t->nodes[node_id];
    int left_count = 0;
    int right_count = 0;
    int left_allocated = INIT_LIST_SIZE;
    int right_allocated = INIT_LIST_SIZE;
    Line* left_list = malloc(sizeof(Line) * INIT_LIST_SIZE);
    Line* right_list = malloc(sizeof(Line) * INIT_LIST_SIZE);
    int pivot_address = rand() % line_count;
    Line pivot = lines[pivot_address];
    push_line(&n->lines, pivot, &n->size_allocated, &n->line_count);
    for(int i = 0; i < line_count; i++) {
        if(i == pivot_address) {
            continue;
        }
        Line i_line = lines[i];
        if(is_on(pivot, i_line)) {
            push_line(&n->lines, i_line, &n->size_allocated, &n->line_count);
        } else if (is_in_front_of(pivot, i_line)) {
            push_line(&left_list, i_line, &left_allocated, &left_count);
        } else if (is_behind(pivot, i_line)) {
            push_line(&right_list, i_line, &right_allocated, &right_count);
        } else {
            // Line is split by the pivot, divide it into front and back segments
            Point2 front_point;
            Point2 back_point;
            if (point_is_in_front_of(pivot, i_line.start)) {
                front_point = i_line.start;
                back_point = i_line.end;
            } else { 
                front_point = i_line.end;
                back_point = i_line.start;
            }
            Point2 intersection = calculate_intersection(pivot, i_line);

            Line front_line;
            front_line.start = front_point;
            front_line.end = intersection;
            front_line.front_sector = i_line.front_sector;
            front_line.back_sector = i_line.back_sector;

            Line back_line;
            back_line.front_sector = i_line.front_sector;
            back_line.back_sector = i_line.back_sector;
            back_line.start = intersection;
            back_line.end = back_point;
 
            push_line(&left_list, front_line, &left_allocated, &left_count);
            push_line(&right_list, back_line, &right_allocated, &right_count);
        }
    }
    recurse_one_side(left_count , node_id, t, true , left_list , left_count, pivot.front_sector);
    recurse_one_side(right_count, node_id, t, false, right_list, right_count, pivot.back_sector);
}

int push_node(Tree* t, Node n) {
    if (t->node_count >= t->node_size_allocated) {
        int new_size = t->node_size_allocated * GROWTH_FACTOR;
        fflush(stdout);
        Node* new_list = realloc(t->nodes, sizeof(Node) * new_size);
        if (new_list == NULL) {
            perror("Failed to reallocate memory for node list");
            exit(EXIT_FAILURE);
        }
        t->nodes = new_list;
        t->node_size_allocated = new_size;
    }
    t->nodes[t->node_count] = n;
    t->node_count += 1;
    return t->node_count - 1;
}

int push_subsector(Tree* t, Subsector s) {
    if (t->subsector_count >= t->subsector_size_allocated) {
        int new_size = t->subsector_size_allocated * GROWTH_FACTOR;
        Subsector* new_list = realloc(t->subsectors, sizeof(Subsector) * new_size);
        if (new_list == NULL) {
            perror("Failed to reallocate memory for node list");
            exit(EXIT_FAILURE);
        }
        t->subsectors = new_list;
        t->subsector_size_allocated = new_size;
    }
    t->subsectors[t->subsector_count] = s;
    t->subsector_count += 1;
    return t->subsector_count - 1;
}

int push_line(Line** list, Line l, int* allocated_size, int* line_count) {
    if (*line_count >= *allocated_size) {
        int new_size = *allocated_size * GROWTH_FACTOR;
        *list = realloc(*list, sizeof(Line) * new_size);
        if (*list == NULL) {
            perror("Failed to reallocate memory for line list");
            exit(EXIT_FAILURE);
        }
        *allocated_size = new_size;
    }
    (*list)[*line_count] = l;
    *line_count += 1;
    return *line_count - 1;
}

void destroy_tree(Tree* t) {
    for(int i = 0; i < t->node_count; i++) {
        free(t->nodes[i].lines);
    }
    free(t->nodes);
    for(int i = 0; i < t->subsector_count; i++) {
        free(t->subsectors[i].region);
    }
    free(t->subsectors);
}