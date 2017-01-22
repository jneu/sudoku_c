#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stdint.h>

typedef uint8_t value_t;
#define UNKNOWN_VALUE ((value_t) 0)

typedef struct def_grid grid;

void grid_create (grid ** g);
void grid_destroy (grid * g);

void grid_clear (grid * g);

void grid_add_given_value (grid * g, int row, int column, value_t value);
void grid_add_given_value_at_index (grid * g, int index, value_t value);
void grid_add_given_exclusion (grid * g, int row, int column, value_t exclusion);

void grid_solve (grid * g);
bool grid_is_solved (const grid * g);
bool grid_is_consistent (const grid * g);

void grid_pretty_print (const grid * g);

#endif
