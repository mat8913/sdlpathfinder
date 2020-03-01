// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2020 Matthew Harm Bekkema <id@mbekkema.name>.
 */

#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "grid.h"
#include <stddef.h>

enum node_color {
	RED_NODE,
	YELLOW_NODE,
	GREEN_NODE
};

struct pathfinder;

struct pathfinder *pathfinder_init(struct grid *grid,
                                   size_t start_x, size_t start_y,
                                   size_t end_x, size_t end_y);

void pathfinder_cleanup(struct pathfinder *pf);

void pathfinder_step(struct pathfinder *pf);

enum node_color pathfinder_node_color(const struct pathfinder *pf,
                                      size_t x, size_t y);

#endif /* PATHFINDER_H */
