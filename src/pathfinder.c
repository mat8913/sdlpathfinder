// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2020 Matthew Harm Bekkema <id@mbekkema.name>.
 *
 * pathfinder.c - The pathfinder implementation.
 *
 * Currently uses the A* search algorithm with the Manhattan distance as the
 * heuristic. A binary heap is used for the priority queue.
 *
 * Some naming conventions:
 *    - `coord` for an index into the `nodes` array (see `get_coord`)
 *    - `hix`   for an index into the `heap`
 */

#include "pathfinder.h"
#include "grid.h"
#include "debug.h"
#include <stdlib.h>

#define INVALID_PATH_SUM ((unsigned int)-1)
#define INVALID_INDEX    ((size_t)-1)

struct node {
	unsigned int path_sum;
	size_t hix;
};

struct pathfinder {
	size_t *heap;
	size_t heap_size;
	size_t goal_x, goal_y;
	const struct grid *grid;
	struct node nodes[];
	/* `heap`  is: size_t heap[width*height];
	 * `nodes` is: struct node nodes[width*height];
	 */
};


/* Utility functions */

static size_t diff(size_t a, size_t b)
{
	return a < b ? b - a : a - b;
}

static size_t get_coord(const struct grid *grid, size_t x, size_t y)
{
	return grid_width(grid)*y + x;
}

static size_t *get_surround(const struct grid *grid, size_t coord)
{
	static size_t ret[5];
	size_t y = coord / grid_width(grid);
	size_t x = coord % grid_width(grid);
	const struct cell_walls walls = grid_cell_walls(grid, x, y);

	size_t i = 0;

	if (!walls.left)
		ret[i++] = get_coord(grid, x-1, y);
	if (!walls.right)
		ret[i++] = get_coord(grid, x+1, y);
	if (!walls.upper)
		ret[i++] = get_coord(grid, x, y-1);
	if (!walls.lower)
		ret[i++] = get_coord(grid, x, y+1);

	ret[i++] = INVALID_INDEX;
	return ret;
}

static enum node_color node_color(const struct node *node)
{
	if (node->hix == INVALID_INDEX) {
		if (node->path_sum == INVALID_PATH_SUM)
			return RED_NODE;
		return GREEN_NODE;
	}
	assert(node->path_sum != INVALID_PATH_SUM);
	return YELLOW_NODE;
}


/* Heap functions */

static size_t heap_weight(const struct pathfinder *pf, size_t coord)
{
	size_t x = coord % grid_width(pf->grid);
	size_t y = coord / grid_width(pf->grid);

	return pf->nodes[coord].path_sum + diff(x, pf->goal_x) + diff(y, pf->goal_y);
}

static void heap_swap(struct pathfinder *pf, size_t hix1, size_t hix2)
{
	const size_t coord1 = pf->heap[hix1];
	const size_t coord2 = pf->heap[hix2];

	pf->heap[hix1] = coord2;
	pf->heap[hix2] = coord1;

	pf->nodes[pf->heap[hix1]].hix = hix1;
	pf->nodes[pf->heap[hix2]].hix = hix2;
}

static bool heap_compare(struct pathfinder *pf, size_t hix1, size_t hix2)
{
	const size_t coord1 = pf->heap[hix1];
	const size_t coord2 = pf->heap[hix2];
	return heap_weight(pf, coord1) <= heap_weight(pf, coord2);
}

static void sift_up(struct pathfinder *pf, const size_t hix)
{
	const size_t parent_hix = (hix-1)/2;

	if (hix == 0 || !heap_compare(pf, hix, parent_hix))
		return;

	heap_swap(pf, hix, parent_hix);
	sift_up(pf, parent_hix);
}

static void sift_down(struct pathfinder *pf, const size_t hix)
{
	const size_t left_hix = 2*hix + 1;
	const size_t right_hix = 2*hix + 2;
	size_t swap_hix = hix;

	if (left_hix < pf->heap_size && heap_compare(pf, left_hix, swap_hix))
		swap_hix = left_hix;

	if (right_hix < pf->heap_size && heap_compare(pf, right_hix, swap_hix))
		swap_hix = right_hix;

	if (swap_hix == hix)
		return;

	heap_swap(pf, hix, swap_hix);
	sift_down(pf, swap_hix);
}

static void update_path_sum(struct pathfinder *pf, size_t coord, size_t path_sum)
{
	struct node *node = &pf->nodes[coord];
	if (node_color(node) != GREEN_NODE && path_sum < node->path_sum) {
		node->path_sum = path_sum;
		if (node->hix == INVALID_INDEX) {
			pf->heap[pf->heap_size] = coord;
			node->hix = pf->heap_size++;
		}
		sift_up(pf, node->hix);
		assert(node_color(node) == YELLOW_NODE);
	}
}

static size_t next_green_coord(struct pathfinder *pf)
{
	assert(pf->heap_size != 0);
	const size_t ret = pf->heap[0];
	assert(pf->nodes[ret].hix == 0);

	pf->heap[0] = pf->heap[--pf->heap_size];
	pf->nodes[pf->heap[0]].hix = 0;
	pf->nodes[ret].hix = INVALID_INDEX;

	sift_down(pf, 0);
	return ret;
}


/* Public functions */

struct pathfinder *pathfinder_init(struct grid *grid,
                                   size_t start_x, size_t start_y,
                                   size_t end_x, size_t end_y)
{
	const size_t width  = grid_width(grid);
	const size_t height = grid_height(grid);
	const size_t start_coord = get_coord(grid, start_x, start_y);

	struct pathfinder *ret = malloc(sizeof(*ret) + width*height * sizeof(*ret->nodes));
	if (!ret) {
		perror("malloc");
		abort();
	}

	ret->goal_x = end_x;
	ret->goal_y = end_y;

	for (size_t coord = 0; coord < width*height; ++coord) {
		ret->nodes[coord].path_sum = INVALID_PATH_SUM;
		ret->nodes[coord].hix = INVALID_INDEX;
	}

	ret->heap = malloc(width*height * sizeof(*ret->heap));
	if (!ret->heap) {
		perror("malloc");
		abort();
	}

	ret->heap[0] = start_coord;
	ret->heap_size = 1;

	ret->nodes[start_coord].path_sum = 0;
	ret->nodes[start_coord].hix = 0;

	ret->grid = grid;
	return ret;
}

void pathfinder_cleanup(struct pathfinder *pf)
{
	free(pf->heap);
	free(pf);
}

void pathfinder_step(struct pathfinder *pf)
{
	if (!pf->heap_size)
		return;

	size_t coord = next_green_coord(pf);
	const unsigned int path_sum = pf->nodes[coord].path_sum + 1;

	const size_t *surround = get_surround(pf->grid, coord);
	for (size_t i = 0; surround[i] != INVALID_INDEX; ++i)
		update_path_sum(pf, surround[i], path_sum);
}

enum node_color pathfinder_node_color(const struct pathfinder *pf,
                                      size_t x, size_t y)
{
	return node_color(&pf->nodes[get_coord(pf->grid, x, y)]);
}
