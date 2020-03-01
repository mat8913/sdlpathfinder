// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2020 Matthew Harm Bekkema <id@mbekkema.name>.
 */

#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stdio.h>

struct grid;

struct cell_walls {
	bool right;
	bool lower;
	bool left;
	bool upper;
};

struct grid *grid_init(size_t width, size_t height);

void grid_cleanup(struct grid *g);

bool *grid_right_wall(struct grid *g, size_t x, size_t y);

bool *grid_lower_wall(struct grid *g, size_t x, size_t y);

bool *grid_left_wall(struct grid *g, size_t x, size_t y);

bool *grid_upper_wall(struct grid *g, size_t x, size_t y);

struct cell_walls grid_cell_walls(const struct grid *g, size_t x, size_t y);

size_t grid_width(const struct grid *g);

size_t grid_height(const struct grid *g);

#endif /* GRID_H */
