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

static void
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

static void
grid_algo_only_one_available_in_cell (grid * g, int index)
{
  value_t i;
  value_t available;

  if (UNKNOWN_VALUE != VALUE (g, index))
    return;

  available = UNKNOWN_VALUE;

  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDED (g, index, i))
        {
          if (UNKNOWN_VALUE != available)
            return;

          available = i;
        }
    }

  grid_set_value_at_index (g, index, available);
}

static void
grid_algo_need_one_or_bounded_in_rowz (grid * g, int rowz, value_t value)
{
  int i;
  int first, last;

  first = -1;
  last = -1;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      if (value == VALUE (g, i))
        return;

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
      g->inconsistent = true;
      return;
    }

  if (last < 0)
    {
      grid_set_value_at_index (g, first, value);
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
                  for (k = 0; k < 3; k++, start++)
                    grid_set_exclusion_at_index (g, start, value);

                  start += 6;
                }
            }
        }
    }
}

static void
grid_algo_need_one_or_bounded_in_colz (grid * g, int colz, value_t value)
{
  int i;
  int first, last;

  first = -1;
  last = -1;

  for (i = colz; i < 81; i += 9)
    {
      if (value == VALUE (g, i))
        return;

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
      g->inconsistent = true;
      return;
    }

  if (last < 0)
    {
      grid_set_value_at_index (g, first, value);
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

          for (j = 0; j < 3; j++, start += 6)
            {
              for (k = 0; k < 3; k++, start++)
                {
                  if (k == first_box_colz)
                    continue;

                  grid_set_exclusion_at_index (g, start, value);
                }
            }
        }
    }
}

static void
grid_algo_need_one_or_bounded_in_box (grid * g, int box, value_t value)
{
  int i, j;
  int start;
  int indices[9];
  int num_indices;

  num_indices = 0;
  start = BOX_START_FROM_WHICH (box);

  for (i = 0; i < 3; i++, start += 6)
    {
      for (j = 0; j < 3; j++, start++)
        {
          if (value == VALUE (g, start))
            return;

          if (!EXCLUDED (g, start, value))
            indices[num_indices++] = start;
        }
    }

  if (0 == num_indices)
    {
      g->inconsistent = true;
      return;
    }

  if (1 == num_indices)
    {
      grid_set_value_at_index (g, indices[0], value);
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
                    grid_set_exclusion_at_index (g, i, value);
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
                    grid_set_exclusion_at_index (g, i, value);
                }
            }
        }
    }
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
