#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grid_internal.h"

int
BOX_START (int index)
{
  int x;

  x = index - (index % 3);
  x -= ((x / 9) % 3) * 9;

  return x;
}

int
BOX_START_FROM_WHICH (int which)
{
  int x;

  x = (which % 3) * 3;
  x += ((which / 3) * 3) * 9;

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

void
grid_set_exclusion_at_index (grid * g, int index, value_t value)
{
  value_t i;
  bool found_non_excluded;

  if (EXCLUDED (g, index, value))
    return;

  if (value == VALUE (g, index))
    {
      g->inconsistent = true;
      return;
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
    g->inconsistent = true;
}

void
grid_set_value_at_index (grid * g, int index, value_t value)
{
  value_t i;
  int j, k;
  int start;
  value_t cur_value = VALUE (g, index);

  if (value == cur_value)
    return;

  if (UNKNOWN_VALUE != cur_value)
    {
      g->inconsistent = true;
      return;
    }

  if (EXCLUDED (g, index, value))
    {
      g->inconsistent = true;
      return;
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
        grid_set_exclusion_at_index (g, i, value);
    }

  start = index % 9;
  for (i = start; i < 81; i += 9)
    {
      if (i != index)
        grid_set_exclusion_at_index (g, i, value);
    }

  start = BOX_START (index);
  for (j = 0; j < 3; j++, start += 6)
    {
      for (k = 0; k < 3; k++, start++)
        {
          if (start == index)
            continue;

          grid_set_exclusion_at_index (g, start, value);
        }
    }
}

static void
grid_set_value (grid * g, int rowz, int colz, value_t value)
{
  grid_set_value_at_index (g, RCZ_2_INDEX (rowz, colz), value);
}

static void
grid_set_exclusion (grid * g, int rowz, int colz, value_t exclusion)
{
  grid_set_exclusion_at_index (g, RCZ_2_INDEX (rowz, colz), exclusion);
}

void
grid_add_given_value (grid * g, int row, int column, value_t value)
{
  if ((row < 1) || (row > 9))
    return;

  if ((column < 1) || (column > 9))
    return;

  if ((value < 1) || (value > 9))
    return;

  grid_set_value (g, ROW_2_ROWZ (row), COL_2_COLZ (column), value);
}

void
grid_add_given_value_at_index (grid * g, int index, value_t value)
{
  if ((index < 0) || (index >= 81))
    return;

  if ((value < 1) || (value > 9))
    return;

  grid_set_value_at_index (g, index, value);
}

void
grid_add_given_exclusion (grid * g, int row, int column, value_t exclusion)
{
  if ((row < 1) || (row > 9))
    return;

  if ((column < 1) || (column > 9))
    return;

  if ((exclusion < 1) || (exclusion > 9))
    return;

  grid_set_exclusion (g, ROW_2_ROWZ (row), COL_2_COLZ (column), exclusion);
}

void
grid_solve (grid * g)
{
  while (g->dirty && !g->inconsistent)
    {
      int i;

      do
        {
          g->dirty = false;

          for (i = 0; i < 81; i++)
            grid_algo_only_one_available_in_cell (g, i);

          for (i = 0; i < 9; i++)
            {
              value_t v;

              for (v = 1; v <= 9; v++)
                {
                  grid_algo_need_one_or_bounded_in_rowz (g, i, v);
                  grid_algo_need_one_or_bounded_in_colz (g, i, v);
                  grid_algo_need_one_or_bounded_in_box (g, i, v);
                }
            }
        }
      while (g->dirty);

      for (i = 0; i < 9; i++)
        {
          grid_algo_pigeon_vacant_in_rowz (g, i);
          grid_algo_pigeon_vacant_in_colz (g, i);
          grid_algo_pigeon_vacant_in_box (g, i);
        }
    }
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

bool
grid_is_solved (const grid * g)
{
  int i;

  if (g->inconsistent)
    return false;

  for (i = 0; i < 81; i++)
    {
      if (UNKNOWN_VALUE == VALUE (g, i))
        return false;
    }

  return true;
}

bool
grid_is_consistent (const grid * g)
{
  if (g->inconsistent)
    return false;

  return true;
}
