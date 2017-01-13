#ifndef GRID_H
#define GRID_H

#include <stdbool.h>
#include <stdint.h>

#define ROW_2_ROWZ(r) ((r) - 1)
#define COL_2_COLZ(c) ((c) - 1)
#define RCZ_2_X(rowz, colz) ((colz) + ((rowz) * 9))

typedef uint8_t value_t;
#define UNKNOWN_VALUE ((value_t) 0)

typedef struct def_grid grid;

void grid_create (grid ** g);
void grid_destroy (grid * g);

void grid_clear (grid * g);

bool grid_add_given_value (grid * g, int row, int column, value_t value);
bool grid_add_given_value_at_index (grid * g, int index, value_t value);
bool grid_add_given_exclusion (grid * g, int row, int column, value_t exclusion);

bool grid_solve (grid * g);

void grid_pretty_print (const grid * g);

#endif
