#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
};

#define ROW_2_ROWZ(r)             ((r) - 1)
#define COL_2_COLZ(c)             ((c) - 1)
#define RCZ_2_INDEX(rowz, colz)   ((colz) + ((rowz) * 9))
#define VALUE(g, index)           ((g)->cells[(index)].value)
#define VALUE_RCZ(g, rowz, colz)  (VALUE((g), RCZ_2_INDEX((rowz), (colz))))
#define EXCLUDED(g, index, value) ((g)->cells[(index)].excluded[((value) - 1)])

static int
BOX_START (int index)
{
  int x;

  x = index - (index % 3);
  x -= ((x / 9) % 3) * 9;

  return x;
}

void
grid_create (grid ** g)
{
  *g = malloc (sizeof (grid));
  if (NULL == *g)
    {
      perror ("failed to alocate grid");
      exit (EXIT_FAILURE);
    }

  grid_clear (*g);
}

void
grid_destroy (grid * g)
{
  free (g);
}

void
grid_clear (grid * g)
{
  memset (g, 0, sizeof (grid));
}

bool
grid_set_exclusion_at_index (grid * g, int index, value_t value)
{
  value_t i;
  bool found_non_excluded;

  if (EXCLUDED (g, index, value))
    return true;

  if (value == VALUE (g, index))
    {
      printf ("%s: excluding cell value which is already set - index: %d value: %d\n", __FUNCTION__, index,
              (int) value);
      return false;
    }

  EXCLUDED (g, index, value) = true;
  g->dirty = true;

  found_non_excluded = false;
  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDED (g, index, i))
        {
          found_non_excluded = true;
          break;
        }
    }

  if (!found_non_excluded)
    {
      printf ("%s: all values excluded for a cell - index: %d\n", __FUNCTION__, index);
      return false;
    }

  return true;
}

bool
grid_set_value_at_index (grid * g, int index, value_t value)
{
  value_t i;
  int j, k;
  int start;
  bool success;
  value_t cur_value = VALUE (g, index);

  if (value == cur_value)
    return true;

  if (UNKNOWN_VALUE != cur_value)
    {
      printf ("%s: cell value already set - index: %d old value: %d new value: %d\n", __FUNCTION__, index,
              (int) cur_value, (int) value);
      return false;
    }

  if (EXCLUDED (g, index, value))
    {
      printf ("%s: value is excluded at this cell - index: %d value: %d\n", __FUNCTION__, index, (int) value);
      return false;
    }

  VALUE (g, index) = value;
  g->dirty = true;

  for (i = 1; i <= 9; i++)
    {
      if (i != value)
        EXCLUDED (g, index, i) = true;
    }

  start = index - (index % 9);
  for (i = start; i < (start + 9); i++)
    {
      if (i != index)
        {
          success = grid_set_exclusion_at_index (g, i, value);

          if (!success)
            return false;
        }
    }

  start = index % 9;
  for (i = start; i < 81; i += 9)
    {
      if (i != index)
        {
          success = grid_set_exclusion_at_index (g, i, value);

          if (!success)
            return false;
        }
    }

  start = BOX_START (index);
  for (j = 0; j < 3; j++)
    {
      for (k = 0; k < 3; k++)
        {
          if (start != index)
            {
              success = grid_set_exclusion_at_index (g, start, value);

              if (!success)
                return false;
            }

          start++;
        }

      start += 9 - 3;
    }

  return true;
}

bool
grid_set_value (grid * g, int rowz, int colz, value_t value)
{
  return grid_set_value_at_index (g, RCZ_2_INDEX (rowz, colz), value);
}

bool
grid_set_exclusion (grid * g, int rowz, int colz, value_t exclusion)
{
  return grid_set_exclusion_at_index (g, RCZ_2_INDEX (rowz, colz), exclusion);
}

bool
grid_add_given_value (grid * g, int row, int column, value_t value)
{
  if ((row < 1) || (row > 9))
    {
      printf ("%s: row must be between 1 and 9, inclusive - row: %d column: %d value: %d\n", __FUNCTION__, row, column,
              (int) value);
      return false;
    }

  if ((column < 1) || (column > 9))
    {
      printf ("%s: column must be between 1 and 9, inclusive - row: %d column: %d value: %d\n", __FUNCTION__, row,
              column, (int) value);
      return false;
    }

  if ((value < 1) || (value > 9))
    {
      printf ("%s: value must be between 1 and 9, inclusive - row: %d column: %d value: %d\n", __FUNCTION__, row,
              column, (int) value);
      return false;
    }

  return grid_set_value (g, ROW_2_ROWZ (row), COL_2_COLZ (column), value);
}

bool
grid_add_given_value_at_index (grid * g, int index, value_t value)
{
  if ((index < 0) || (index >= 81))
    {
      printf ("%s: index must be between 0 and 81, inclusive - index: %d value: %d\n", __FUNCTION__, index,
              (int) value);
      return false;
    }

  if ((value < 1) || (value > 9))
    {
      printf ("%s: value must be between 1 and 9, inclusive - index: %d value: %d\n", __FUNCTION__, index, (int) value);
      return false;
    }

  return grid_set_value_at_index (g, index, value);
}

bool
grid_add_given_exclusion (grid * g, int row, int column, value_t exclusion)
{
  if ((row < 1) || (row > 9))
    {
      printf ("%s: row must be between 1 and 9, inclusive - row: %d column: %d exclusion: %d\n", __FUNCTION__, row,
              column, (int) exclusion);
      return false;
    }

  if ((column < 1) || (column > 9))
    {
      printf ("%s: column must be between 1 and 9, inclusive - row: %d column: %d exclusion: %d\n", __FUNCTION__, row,
              column, (int) exclusion);
      return false;
    }

  if ((exclusion < 1) || (exclusion > 9))
    {
      printf ("%s: value must be between 1 and 9, inclusive - row: %d column: %d exclusion: %d\n", __FUNCTION__, row,
              column, (int) exclusion);
      return false;
    }

  return grid_set_exclusion (g, ROW_2_ROWZ (row), COL_2_COLZ (column), exclusion);
}

static bool
grid_algo_only_one_available_in_cell (grid * g, int index)
{
  value_t i;
  value_t available;

  if (UNKNOWN_VALUE != VALUE (g, index))
    return true;

  available = UNKNOWN_VALUE;

  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDED (g, index, i))
        {
          if (UNKNOWN_VALUE != available)
            return true;

          available = i;
        }
    }

  return grid_set_value_at_index (g, index, available);
}

static bool
grid_algo_need_one_or_bounded_in_rowz (grid * g, int rowz, value_t value)
{
  int i;
  int first, last;
  bool success;

  first = -1;
  last = -1;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      if (value == VALUE (g, i))
        return true;

      if (!EXCLUDED (g, i, value))
        {
          if (first < 0)
            first = i;
          else
            last = i;
        }
    }

  if (first < 0)
    {
      printf ("%s: value is not allowed in row - rowz: %d value: %d\n", __FUNCTION__, rowz, (int) value);
      return false;
    }

  if (last < 0)
    {
      success = grid_set_value_at_index (g, first, value);

      if (!success)
        return false;
    }
  else
    {
      int first_colz, last_colz;
      bool bounded = false;

      first_colz = first % 9;
      last_colz = last % 9;

      if (first_colz < 3)
        {
          if (last_colz < 3)
            bounded = true;
        }
      else if (first_colz < 6)
        {
          if (last_colz < 6)
            bounded = true;
        }
      else
        {
          bounded = true;
        }

      if (bounded)
        {
          int j, k;
          int start;
          int first_box_rowz;

          start = BOX_START (first);
          first_box_rowz = (first / 9) % 3;

          for (j = 0; j < 3; j++)
            {
              if (j == first_box_rowz)
                {
                  start += 9;
                }
              else
                {
                  for (k = 0; k < 3; k++)
                    {
                      success = grid_set_exclusion_at_index (g, start, value);

                      if (!success)
                        return false;

                      start++;
                    }

                  start += 9 - 3;
                }
            }
        }
    }

  return true;
}

static bool
grid_algo_need_one_or_bounded_in_colz (grid * g, int colz, value_t value)
{
  int i;
  int first, last;
  bool success;

  first = -1;
  last = -1;

  for (i = colz; i < 81; i += 9)
    {
      if (value == VALUE (g, i))
        return true;

      if (!EXCLUDED (g, i, value))
        {
          if (first < 0)
            first = i;
          else
            last = i;
        }
    }

  if (first < 0)
    {
      printf ("%s: value is not allowed in column - colz: %d value: %d\n", __FUNCTION__, colz, (int) value);
      return false;
    }

  if (last < 0)
    {
      success = grid_set_value_at_index (g, first, value);

      if (!success)
        return false;
    }
  else
    {
      int first_rowz, last_rowz;
      bool bounded = false;

      first_rowz = first / 9;
      last_rowz = last / 9;

      if (first_rowz < 3)
        {
          if (last_rowz < 3)
            bounded = true;
        }
      else if (first_rowz < 6)
        {
          if (last_rowz < 6)
            bounded = true;
        }
      else
        {
          bounded = true;
        }

      if (bounded)
        {
          int j, k;
          int start;
          int first_box_colz;

          start = BOX_START (first);
          first_box_colz = first % 3;

          for (j = 0; j < 3; j++)
            {
              for (k = 0; k < 3; k++)
                {
                  if (k != first_box_colz)
                    {
                      success = grid_set_exclusion_at_index (g, start, value);

                      if (!success)
                        return false;
                    }

                  start++;
                }

              start += 9 - 3;
            }
        }
    }

  return true;
}

static bool
grid_algo_need_one_or_bounded_in_box (grid * g, int box, value_t value)
{
  int i, j, k;
  bool success;
  int indices[9];
  int num_indices;

  i = ((box % 3) * 3) + ((box / 3) * 3) * 9;
  num_indices = 0;

  for (j = 0; j < 3; j++)
    {
      for (k = 0; k < 3; k++)
        {
          if (value == VALUE (g, i))
            return true;

          if (!EXCLUDED (g, i, value))
            indices[num_indices++] = i;

          i++;
        }

      i += 9 - 3;
    }

  if (0 == num_indices)
    {
      printf ("%s: value is not allowed in box - box: %d value: %d\n", __FUNCTION__, box, (int) value);
      return false;
    }

  if (1 == num_indices)
    {
      success = grid_set_value_at_index (g, indices[0], value);

      if (!success)
        return false;
    }
  else if (num_indices <= 3)
    {
      int rz1, cz1, rz2, cz2;

      rz1 = indices[0] / 9;
      cz1 = indices[0] % 9;

      rz2 = indices[1] / 9;
      cz2 = indices[1] % 9;

      if (rz1 == rz2)
        {
          if ((2 == num_indices) || (rz1 == (indices[2] / 9)))
            {
              for (i = rz1 * 9; i < (rz1 + 1) * 9; i++)
                {
                  if (((i % 9) / 3) != (box % 3))
                    {
                      success = grid_set_exclusion_at_index (g, i, value);

                      if (!success)
                        return false;
                    }
                }
            }
        }
      else if (cz1 == cz2)
        {
          if ((2 == num_indices) || (cz1 == (indices[2] % 9)))
            {
              for (i = cz1; i < 81; i += 9)
                {
                  if (((i / 9) / 3) != (box / 3))
                    {
                      success = grid_set_exclusion_at_index (g, i, value);

                      if (!success)
                        return false;
                    }
                }
            }
        }
    }

  return true;
}

typedef struct def_block
{
  int items[9];
  int num_items;
} block;

typedef struct def_pigeon_context pigeon_context;
typedef bool (*pigeon_callback) (pigeon_context *);

struct def_pigeon_context
{
  const block *b_orig;
  int subset_card;

  int offset;
  grid *g;

  block b_working;
  bool excluded_working[9];

  pigeon_callback handle_pigeon;
};

static bool
enumerate_subsets_recurse (pigeon_context * context, int depth, int from)
{
  int i;
  int start_num_items;
  bool success;
  const block *b_orig = context->b_orig;
  block *b_working = &context->b_working;

  start_num_items = b_working->num_items++;
  depth--;

  for (i = from; i < b_orig->num_items - depth; i++)
    {
      b_working->items[start_num_items] = b_orig->items[i];

      if (0 == depth)
        {
          int j, k;
          int num_not_fully_excluded;
          cell *c = context->g->cells;
          bool *p = context->excluded_working;

          memcpy (p, c[b_working->items[0]].excluded, 9 * sizeof (bool));

          for (j = 1; j < context->subset_card; j++)
            {
              const bool *p_orig = c[b_working->items[j]].excluded;

              for (k = 0; k < 9; k++)
                p[k] &= p_orig[k];
            }

          num_not_fully_excluded = 0;
          for (j = 0; j < 9; j++)
            if (!p[j])
              num_not_fully_excluded++;

          if (num_not_fully_excluded == context->subset_card)
            {
              success = context->handle_pigeon (context);

              if (!success)
                return false;
            }
        }
      else
        {
          success = enumerate_subsets_recurse (context, depth, i + 1);

          if (!success)
            return false;
        }
    }

  b_working->num_items = start_num_items;

  return true;
}

static bool
enumerate_subsets (grid * g, int offset, int subset_card, const block * b_orig, pigeon_callback handle_pigeon)
{
  pigeon_context context;

  context.b_orig = b_orig;
  context.subset_card = subset_card;
  context.offset = offset;
  context.g = g;
  context.b_working.num_items = 0;
  context.handle_pigeon = handle_pigeon;

  return enumerate_subsets_recurse (&context, subset_card, 0);
}

static bool
pigeon_clear_row (pigeon_context * context)
{
  int i, j;
  bool success;
  int subset_card = context->subset_card;
  int rowz = context->offset;
  grid *g = context->g;
  block *b_working = &context->b_working;
  bool *excluded_working = context->excluded_working;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      bool use_it;

      if (UNKNOWN_VALUE != VALUE (g, i))
        continue;

      use_it = true;

      for (j = 0; j < subset_card; j++)
        {
          if (i == b_working->items[j])
            {
              use_it = false;
              break;
            }
        }

      if (!use_it)
        continue;

      for (j = 0; j < 9; j++)
        {
          if (!excluded_working[j])
            {
              success = grid_set_exclusion_at_index (g, i, (value_t) (j + 1));

              if (!success)
                return false;
            }
        }
    }

  return true;
}

static bool
grid_algo_pigeon_vacant_in_rowz (grid * g, int rowz)
{
  int i;
  int subset_card;
  block vacant;

  vacant.num_items = 0;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      if (UNKNOWN_VALUE == VALUE (g, i))
        vacant.items[vacant.num_items++] = i;
    }

  for (subset_card = 2; subset_card <= (vacant.num_items - 2); subset_card++)
    {
      bool success;

      success = enumerate_subsets (g, rowz, subset_card, &vacant, pigeon_clear_row);

      if (!success)
        return false;
    }

  return true;
}

static bool
pigeon_clear_column (pigeon_context * context)
{
  int i, j;
  bool success;
  int subset_card = context->subset_card;
  int colz = context->offset;
  grid *g = context->g;
  block *b_working = &context->b_working;
  bool *excluded_working = context->excluded_working;

  for (i = colz; i < 81; i += 9)
    {
      bool use_it;

      if (UNKNOWN_VALUE != VALUE (g, i))
        continue;

      use_it = true;

      for (j = 0; j < subset_card; j++)
        {
          if (i == b_working->items[j])
            {
              use_it = false;
              break;
            }
        }

      if (!use_it)
        continue;

      for (j = 0; j < 9; j++)
        {
          if (!excluded_working[j])
            {
              success = grid_set_exclusion_at_index (g, i, (value_t) (j + 1));

              if (!success)
                return false;
            }
        }
    }

  return true;
}

static bool
grid_algo_pigeon_vacant_in_colz (grid * g, int colz)
{
  int i;
  int subset_card;
  block vacant;

  vacant.num_items = 0;

  for (i = colz; i < 81; i += 9)
    {
      if (UNKNOWN_VALUE == VALUE (g, i))
        vacant.items[vacant.num_items++] = i;
    }

  for (subset_card = 2; subset_card <= (vacant.num_items - 2); subset_card++)
    {
      bool success;

      success = enumerate_subsets (g, colz, subset_card, &vacant, pigeon_clear_column);

      if (!success)
        return false;
    }

  return true;
}

static bool
pigeon_clear_box (pigeon_context * context)
{
  int i, j, k;
  int start;
  bool success;
  int subset_card = context->subset_card;
  int box = context->offset;
  grid *g = context->g;
  block *b_working = &context->b_working;
  bool *excluded_working = context->excluded_working;

  start = ((box % 3) * 3) + ((box / 3) * 3) * 9;
  for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 3; j++)
        {
          bool use_it;

          if (UNKNOWN_VALUE != VALUE (g, start))
            continue;

          use_it = true;

          for (j = 0; j < subset_card; j++)
            {
              if (start == b_working->items[j])
                {
                  use_it = false;
                  break;
                }
            }

          if (!use_it)
            continue;

          for (k = 0; k < 9; k++)
            {
              if (!excluded_working[k])
                {
                  success = grid_set_exclusion_at_index (g, start, (value_t) (k + 1));

                  if (!success)
                    return false;
                }
            }

          start++;
        }

      start += 9 - 3;
    }

  return true;
}

static bool
grid_algo_pigeon_vacant_in_box (grid * g, int box)
{
  int i, j;
  int start;
  int subset_card;
  block vacant;

  vacant.num_items = 0;

  start = ((box % 3) * 3) + ((box / 3) * 3) * 9;
  for (i = 0; i < 3; i++)
    {
      for (j = 0; j < 3; j++)
        {
          if (UNKNOWN_VALUE == VALUE (g, start))
            vacant.items[vacant.num_items++] = start;

          start++;
        }

      start += 9 - 3;
    }

  for (subset_card = 2; subset_card <= (vacant.num_items - 2); subset_card++)
    {
      bool success;

      success = enumerate_subsets (g, box, subset_card, &vacant, pigeon_clear_box);

      if (!success)
        return false;
    }

  return true;
}

bool
grid_solve (grid * g)
{
  while (g->dirty)
    {
      int i;
      bool success;

      do
        {
          g->dirty = false;

          for (i = 0; i < 81; i++)
            {
              success = grid_algo_only_one_available_in_cell (g, i);

              if (!success)
                return false;
            }

          for (i = 0; i < 9; i++)
            {
              value_t v;

              for (v = 1; v <= 9; v++)
                {
                  success = grid_algo_need_one_or_bounded_in_rowz (g, i, v);

                  if (!success)
                    return false;

                  success = grid_algo_need_one_or_bounded_in_colz (g, i, v);

                  if (!success)
                    return false;

                  success = grid_algo_need_one_or_bounded_in_box (g, i, v);

                  if (!success)
                    return false;
                }
            }
        }
      while (g->dirty);

      for (i = 0; i < 9; i++)
        {
          success = grid_algo_pigeon_vacant_in_rowz (g, i);

          if (!success)
            return false;

          success = grid_algo_pigeon_vacant_in_colz (g, i);

          if (!success)
            return false;

          success = grid_algo_pigeon_vacant_in_box (g, i);

          if (!success)
            return false;
        }
    }

  return true;
}

#define TERM_RED "\x1B[31m"
#define TERM_RESTORE "\x1B[0m"

void
grid_pretty_print (const grid * g)
{
  int i, j;
  int rowz, colz;

  printf ("\n");

  for (rowz = 0; rowz < 9; rowz++)
    {
      for (i = 0; i < 3; i++)
        {
          for (colz = 0; colz < 9; colz++)
            {
              if (UNKNOWN_VALUE != VALUE_RCZ (g, rowz, colz))
                {
                  if (1 == i)
                    printf (" %d ", (int) VALUE_RCZ (g, rowz, colz));
                  else
                    printf ("   ");
                }
              else
                {
                  for (j = 1; j <= 3; j++)
                    {
                      if (EXCLUDED (g, RCZ_2_INDEX (rowz, colz), j + 3 * i))
                        printf (" ");
                      else
                        printf (TERM_RED "%d" TERM_RESTORE, j + 3 * i);
                    }
                }

              if (8 != colz)
                {
                  if (2 == (colz % 3))
                    printf ("#");
                  else
                    printf ("|");
                }
            }

          printf ("\n");
        }

      if (8 != rowz)
        {
          if (2 == (rowz % 3))
            printf ("###################################\n");
          else
            printf ("-----------#-----------#-----------\n");
        }
    }

  printf ("\n");
}
