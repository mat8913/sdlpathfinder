// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2020 Matthew Harm Bekkema <id@mbekkema.name>.
 */

#include "pathfinder.h"
#include "grid.h"
#include "debug.h"

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>

/* Default grid dimensions
 * TODO: Allow user to resize
 */
#define GRID_WIDTH 40
#define GRID_HEIGHT 32

#define BOX_SIZE 20
#define SCREEN_WIDTH (GRID_WIDTH * BOX_SIZE)
#define SCREEN_HEIGHT (GRID_HEIGHT * BOX_SIZE)

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static struct grid *grid = NULL;
static struct pathfinder *pf = NULL;
static bool quit = false;

static SDL_Rect box_get_screen_rect(int x, int y)
{
	SDL_Rect ret;
	ret.x = x * BOX_SIZE;
	ret.y = y * BOX_SIZE;
	ret.w = BOX_SIZE;
	ret.h = BOX_SIZE;
	return ret;
}

/* Create initial walls
 * TODO: Allow user to add/remove walls
 */
static void init_walls(void)
{
	for (size_t x = 1; x < GRID_WIDTH; ++x)
		*grid_upper_wall(grid, x, 10) = true;

	for (size_t x = 0; x < GRID_HEIGHT-1; ++x)
		*grid_upper_wall(grid, x, 13) = true;

	*grid_upper_wall(grid, 15, 15) = true;
	*grid_upper_wall(grid, 16, 15) = true;
	*grid_upper_wall(grid, 17, 15) = true;
	*grid_upper_wall(grid, 18, 15) = true;
	*grid_upper_wall(grid, 19, 15) = true;
	*grid_right_wall(grid, 19, 15) = true;
	*grid_right_wall(grid, 19, 15) = true;
	*grid_right_wall(grid, 19, 16) = true;
	*grid_right_wall(grid, 19, 17) = true;
	*grid_right_wall(grid, 19, 18) = true;
	*grid_right_wall(grid, 19, 19) = true;
}

static void init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
		abort();
	}

	window = SDL_CreateWindow("SDL Path Finder", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (window == NULL) {
		fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
		abort();
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
#ifndef DEBUG_RENDER_SPEED
		| SDL_RENDERER_PRESENTVSYNC
#endif /* DEBUG_RENDER_SPEED */
		);
	if (!renderer) {
		fprintf(stderr, "SDL_CreateRenderer: %s\n", SDL_GetError());
		abort();
	}

	grid = grid_init(GRID_WIDTH, GRID_HEIGHT);
	/* TODO: Allow user to select start/end */
	pf = pathfinder_init(grid, GRID_WIDTH-1, 0, 19, 15);
	init_walls();
}

static void cleanup(void)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	pathfinder_cleanup(pf);
	grid_cleanup(grid);
}

static void handle_events(void)
{
	SDL_Event e;

	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			quit = true;
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
			case SDLK_q:
				quit = true;
				return;
			case SDLK_s:
				pathfinder_step(pf);
				break;
			}
		}
	}
}

static void render_walls(void)
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
	for (int y = 0; y < GRID_HEIGHT; ++y) {
		for (int x = 0; x < GRID_WIDTH; ++x) {
			if (x < GRID_WIDTH-1 && *grid_right_wall(grid, x, y))
				SDL_RenderDrawLine(renderer, (x+1) * BOX_SIZE, y * BOX_SIZE, (x+1) * BOX_SIZE, y * BOX_SIZE + BOX_SIZE);

			if (y < GRID_HEIGHT-1 && *grid_lower_wall(grid, x, y))
				SDL_RenderDrawLine(renderer, x * BOX_SIZE, (y+1) * BOX_SIZE, x * BOX_SIZE + BOX_SIZE, (y+1) * BOX_SIZE);
		}
	}
}

static void render_grid(void)
{
	SDL_SetRenderDrawColor(renderer, 0xD0, 0xD0, 0xD0, 0xFF);
	for (int x = 1; x < GRID_WIDTH; ++x)
		SDL_RenderDrawLine(renderer, x * BOX_SIZE, 0, x * BOX_SIZE, SCREEN_HEIGHT);
	for (int y = 1; y < GRID_HEIGHT; ++y)
		SDL_RenderDrawLine(renderer, 0, y * BOX_SIZE, SCREEN_WIDTH, y * BOX_SIZE);
}

static void render_pathfinder(void)
{
	for (size_t x = 0; x < GRID_WIDTH; ++x) {
		for (size_t y = 0; y < GRID_HEIGHT; ++y) {
			switch (pathfinder_node_color(pf, x, y)) {
			case YELLOW_NODE:
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);
				break;
			case GREEN_NODE:
				SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
				break;
			case RED_NODE:
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
				break;
			}
			SDL_Rect rect = box_get_screen_rect(x, y);
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

static void render_frame(void)
{
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(renderer);

	render_pathfinder();
	render_grid();
	render_walls();

	SDL_RenderPresent(renderer);
}

int main(void)
{
	print_debug_status();
	init();

	while (!quit) {
#ifdef DEBUG_RENDER_SPEED
		const uint32_t start_ticks = SDL_GetTicks();
#endif /* DEBUG_RENDER_SPEED */

		handle_events();
		render_frame();

#ifdef DEBUG_RENDER_SPEED
		const uint32_t end_ticks = SDL_GetTicks();
		if (end_ticks - start_ticks > 16)
			fprintf(stderr, "Took too long to render: %dms\n", end_ticks - start_ticks);
#endif /* DEBUG_RENDER_SPEED */
	}

	cleanup();
	return 0;
}
