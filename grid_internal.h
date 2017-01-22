#ifndef GRID_INTERNAL_H
#define GRID_INTERNAL_H

#include "grid.h"

typedef struct def_cell
{
  value_t value;
  bool excluded[9];
} cell;

struct def_grid
{
  cell cells[81];
  bool dirty;
  bool inconsistent;
};

#define ROW_2_ROWZ(r)             ((r) - 1)
#define COL_2_COLZ(c)             ((c) - 1)
#define RCZ_2_INDEX(rowz, colz)   ((colz) + ((rowz) * 9))
#define VALUE(g, index)           ((g)->cells[(index)].value)
#define VALUE_RCZ(g, rowz, colz)  (VALUE((g), RCZ_2_INDEX((rowz), (colz))))
#define EXCLUDED(g, index, value) ((g)->cells[(index)].excluded[((value) - 1)])

int BOX_START (int index);
int BOX_START_FROM_WHICH (int which);

void grid_set_exclusion_at_index (grid * g, int index, value_t value);

void grid_algo_pigeon_vacant_in_rowz (grid * g, int rowz);
void grid_algo_pigeon_vacant_in_colz (grid * g, int colz);
void grid_algo_pigeon_vacant_in_box (grid * g, int box);

#endif
