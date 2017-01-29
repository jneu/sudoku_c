#ifndef GRID_INTERNAL_H
#define GRID_INTERNAL_H

/*
 * grid_internal.h
 * Header to be included only by internal sources.
 */

#include "grid.h"

/*
 * Basic cell...
 */
typedef struct def_cell
{
  value_t value;
  bool excluded[9];
} cell;

/*
 * ... of which 81 make a grid (or puzzle).
 */
struct def_grid
{
  cell cells[81];
  bool dirty;
  bool inconsistent;
};

/*
 * For clarity, we use macros to simplify common access patterns.
 */

#define ROW_2_ROWZ(r)             ((r) - 1)
#define COL_2_COLZ(c)             ((c) - 1)
#define RCZ_2_INDEX(rowz, colz)   ((colz) + ((rowz) * 9))
#define VALUE(g, index)           ((g)->cells[(index)].value)
#define VALUE_RCZ(g, rowz, colz)  (VALUE((g), RCZ_2_INDEX((rowz), (colz))))
#define EXCLUDED(g, index, value) ((g)->cells[(index)].excluded[((value) - 1)])

int BOX_START (int index);
int BOX_START_FROM_WHICH (int which);

void grid_set_value_at_index (grid * g, int index, value_t value);
void grid_set_exclusion_at_index (grid * g, int index, value_t value);

/*
 * Pull in the algorithm prototypes here so they can be used by grid.c.
 */

void grid_algo_only_one_available_in_cell (grid * g, int index);
void grid_algo_need_one_or_bounded_in_rowz (grid * g, int rowz, value_t value);
void grid_algo_need_one_or_bounded_in_colz (grid * g, int colz, value_t value);
void grid_algo_need_one_or_bounded_in_box (grid * g, int box, value_t value);

void grid_algo_pigeon_vacant_in_rowz (grid * g, int rowz);
void grid_algo_pigeon_vacant_in_colz (grid * g, int colz);
void grid_algo_pigeon_vacant_in_box (grid * g, int box);

#endif
