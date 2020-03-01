// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Copyright (C) 2020 Matthew Harm Bekkema <id@mbekkema.name>.
 *
 * debug.h - Debugging and assert macros
 *
 * Include this instead of <assert.h>.
 *
 * Define the following macros when compiling to enable certain debugging
 * features:
 *
 *    DEBUG: Enable assert statements
 *
 *    DEBUG_RENDER_SPEED: Disable vsync and print when rendering takes too long
 */

#ifdef DEBUG
#undef NDEBUG
#else
#define NDEBUG
#endif

#include <assert.h>
#include <stdio.h>

static inline void print_debug_status(void)
{
	#ifdef DEBUG
	fprintf(stderr, "DEBUG enabled\n");
	#endif

	#ifdef DEBUG_RENDER_SPEED
	fprintf(stderr, "DEBUG_RENDER_SPEED enabled\n");
	#endif
}
