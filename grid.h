#ifndef GRID_H
#define GRID_H

/*
 * grid.h
 * Public grid API.
 */

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t value_t;
#define UNKNOWN_VALUE ((value_t) 0)

/*
 * Opaque grid stricture.
 */
typedef struct def_grid grid;

/*
 * Create or destroy a grid structure. grid_destroy needs to be called only to
 * free memory allocated by grid_create.
 */
void grid_create (grid ** g);
void grid_destroy (grid * g);

/*
 * Reset a grid back to its original state, with no values set and no
 * exceptions.
 */
void grid_clear (grid * g);

void grid_add_given_value (grid * g, int row, int column, value_t value);
void grid_add_given_value_at_index (grid * g, int index, value_t value);
void grid_add_given_exclusion (grid * g, int row, int column, value_t exclusion);

/*
 * grid_solve runs all algorithms until no further changes are possible.
 */
void grid_solve (grid * g);
bool grid_is_solved (const grid * g);
bool grid_is_consistent (const grid * g);

/*
 * Print out a grid in its current state.
 */
void grid_pretty_print (const grid * g);

#endif
