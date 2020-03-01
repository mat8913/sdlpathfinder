// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2020 Matthew Harm Bekkema <id@mbekkema.name>.
 */

#include "grid.h"
#include "debug.h"
#include <stdbool.h>
#include <stdlib.h>

struct grid {
	size_t width, height;
	bool walls[];
	/* `walls` is:
	 * bool right_walls[(width-1) * height];
	 * bool lower_walls[width * (height-1)];
	 */
};

struct grid *grid_init(size_t width, size_t height)
{
	/* calloc is used to init all walls to `false` */
	struct grid *ret = calloc(sizeof(*ret) + (2*width*height - width - height) * sizeof(*ret->walls), 1);
	if (!ret) {
		perror("calloc");
		abort();
	}

	ret->width = width;
	ret->height = height;
	return ret;
}

void grid_cleanup(struct grid *g)
{
	free(g);
}

bool *grid_right_wall(struct grid *g, size_t x, size_t y)
{
	/* Right wall of the final column is presumed */
	assert(x < g->width - 1);
	assert(y < g->height);

	return &g->walls[(g->width-1) * y + x];
}

bool *grid_lower_wall(struct grid *g, size_t x, size_t y)
{
	/* Lower wall of the final row is presumed */
	assert(y < g->height - 1);
	assert(x < g->width);

	bool *lower_walls = g->walls + (g->width-1)*g->height;
	return &lower_walls[g->width * y + x];
}

bool *grid_left_wall(struct grid *g, size_t x, size_t y)
{
	/* Left wall of the first column is presumed */
	assert(x > 0);
	return grid_right_wall(g, x-1, y);
}

bool *grid_upper_wall(struct grid *g, size_t x, size_t y)
{
	/* Upper wall of the first row is presumed */
	assert(y > 0);
	return grid_lower_wall(g, x, y-1);
}

struct cell_walls grid_cell_walls(const struct grid *cg, size_t x, size_t y)
{
	struct grid *g = (struct grid *)cg;
	struct cell_walls ret;
	ret.right = x == g->width  - 1 || *grid_right_wall(g, x, y);
	ret.lower = y == g->height - 1 || *grid_lower_wall(g, x, y);
	ret.left  = x == 0             || *grid_left_wall(g, x, y);
	ret.upper = y == 0             || *grid_upper_wall(g, x, y);
	return ret;
}

size_t grid_width(const struct grid *g)
{
	return g->width;
}

size_t grid_height(const struct grid *g)
{
	return g->height;
}
