#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
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

#define VALUEX(g, index) ((g)->cells[(index)].value)
#define VALUE(g, rowz, colz) (VALUEX((g), RCZ_2_X(rowz, colz)))
#define EXCLUDEDX(g, index, value) ((g)->cells[(index)].excluded[((value) - 1)])

static int
grid_box_start_index (int index)
{
  int box_index;

  box_index = index - index % 3;
  box_index -= ((box_index / 9) % 3) * 9;

  return box_index;
}

bool
grid_set_exclusion_at_index (grid * g, int index, value_t value)
{
  value_t i;

  if (EXCLUDEDX (g, index, value))
    return true;

  if (value == VALUEX (g, index))
    {
      printf ("%s: excluding cell value which is already set - index: %d value: %d\n", __FUNCTION__, index,
              (int) value);
      return false;
    }

  EXCLUDEDX (g, index, value) = true;
  g->dirty = true;

  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDEDX (g, index, i))
        break;
    }

  if (10 == i)
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
  value_t cur_value = VALUEX (g, index);

  if (value == cur_value)
    return true;

  if (UNKNOWN_VALUE != cur_value)
    {
      printf ("%s: cell value already set - index: %d old value: %d new value: %d\n", __FUNCTION__, index,
              (int) cur_value, (int) value);
      return false;
    }

  if (EXCLUDEDX (g, index, value))
    {
      printf ("%s: value is excluded at this cell - index: %d value: %d\n", __FUNCTION__, index, (int) value);
      return false;
    }

  VALUEX (g, index) = value;
  g->dirty = true;

  for (i = 1; i <= 9; i++)
    {
      if (i != value)
        EXCLUDEDX (g, index, i) = true;
    }

  start = index - index % 9;
  for (i = start; i < start + 9; i++)
    {
      if (i != index)
        {
          if (!grid_set_exclusion_at_index (g, i, value))
            return false;
        }
    }

  start = index % 9;
  for (i = start; i < 81; i += 9)
    {
      if (i != index)
        {
          if (!grid_set_exclusion_at_index (g, i, value))
            return false;
        }
    }

  start = grid_box_start_index (index);
  for (j = 0; j < 3; j++)
    {
      for (k = 0; k < 3; k++)
        {
          if (start != index)
            {
              if (!grid_set_exclusion_at_index (g, start, value))
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
  return grid_set_value_at_index (g, RCZ_2_X (rowz, colz), value);
}

bool
grid_set_exclusion (grid * g, int rowz, int colz, value_t exclusion)
{
  return grid_set_exclusion_at_index (g, RCZ_2_X (rowz, colz), exclusion);
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

  if (UNKNOWN_VALUE != VALUEX (g, index))
    return true;

  available = UNKNOWN_VALUE;

  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDEDX (g, index, i))
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

  first = -1;
  last = -1;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      if (value == VALUEX (g, i))
        return true;

      if (!EXCLUDEDX (g, i, value))
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
      if (!grid_set_value_at_index (g, first, value))
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

          start = grid_box_start_index (first);
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
                      if (!grid_set_exclusion_at_index (g, start, value))
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

  first = -1;
  last = -1;

  for (i = colz; i < 81; i += 9)
    {
      if (value == VALUEX (g, i))
        return true;

      if (!EXCLUDEDX (g, i, value))
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
      if (!grid_set_value_at_index (g, first, value))
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

          start = grid_box_start_index (first);
          first_box_colz = first % 3;

          for (j = 0; j < 3; j++)
            {
              for (k = 0; k < 3; k++)
                {
                  if (k != first_box_colz)
                    {
                      if (!grid_set_exclusion_at_index (g, start, value))
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

bool
grid_solve (grid * g)
{
  while (g->dirty)
    {
      do
        {
          int i;

          g->dirty = false;

          for (i = 0; i < 81; i++)
            {
              if (!grid_algo_only_one_available_in_cell (g, i))
                return false;
            }

          for (i = 0; i < 9; i++)
            {
              value_t v;

              for (v = 1; v <= 9; v++)
                {
                  if (!grid_algo_need_one_or_bounded_in_rowz (g, i, v))
                    return false;

                  if (!grid_algo_need_one_or_bounded_in_colz (g, i, v))
                    return false;

#if 0
                  /* @@@ is this ever needed? */
                  grid_algo_need_one_or_bounded_in_box (g, i, v);
#endif
                }
            }
        }
      while (g->dirty);

      /* @@@ add in the pidgen hole algos */
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
              if (UNKNOWN_VALUE != VALUE (g, rowz, colz))
                {
                  if (1 == i)
                    printf (" %d ", (int) VALUE (g, rowz, colz));
                  else
                    printf ("   ");
                }
              else
                {
                  for (j = 1; j <= 3; j++)
                    {
                      if (EXCLUDEDX (g, RCZ_2_X (rowz, colz), j + 3 * i))
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
