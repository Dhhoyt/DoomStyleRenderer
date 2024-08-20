#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "constants.h"
#include "geometry.h"
#include "bsp.h"
#include "time.h"
#include "rendering.h"

SDL_Window* window;
SDL_Renderer* renderer;

bool game_is_running = false;

const int FPS = 60;
const int frameDelay = 1000 / FPS;

real_t heading = 0;

bool init_window(void) {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initalizing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        "Binary Space Partitioning Demo",
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        WINDOW_WIDTH * PIXEL_SCALE, 
        WINDOW_HEIGHT * PIXEL_SCALE, 
        SDL_WINDOW_SHOWN
    );
    if (window == NULL) {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }
    return true;
}

void setup() {

}

void process_input(SDL_Event* event){
    switch(event->type) {
        case SDL_QUIT:
            game_is_running = false;
            break;
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == SDLK_ESCAPE) {
                game_is_running = false;
            }
            break;
    }
}

void update() {
    heading += 0.01;
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    Point3 player_pos;
    player_pos.x = 0;
    player_pos.y = 0;
    player_pos.z = 0;
    Point3 p;
    p.x = 10;
    p.y = 30;
    p.z = 10;
    Pixel a = project_point(p, player_pos, heading, FOV);
    p.x = -10;
    Pixel b = project_point(p, player_pos, heading, FOV);
    p.z = 0;
    Pixel c = project_point(p, player_pos, heading, FOV);
    p.x = 10;
    Pixel d = project_point(p, player_pos, heading, FOV);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderDrawLine(renderer, a.x, a.y, b.x, b.y);
    SDL_RenderDrawLine(renderer, a.x, a.y, d.x, d.y);
    SDL_RenderDrawLine(renderer, c.x, c.y, b.x, b.y);
    SDL_RenderDrawLine(renderer, c.x, c.y, d.x, d.y);
    
    SDL_RenderPresent(renderer);
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(void) {
    setup();
    game_is_running = init_window();
    while(game_is_running) {
        Uint32 frameStart = SDL_GetTicks();

        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            process_input(&e);
        }
        update();
        render();

        int frameTime = SDL_GetTicks() - frameStart;
        
        // If the frame was completed too quickly, delay to maintain 60 FPS
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }

    }
    destroy_window();
    return 0;
}