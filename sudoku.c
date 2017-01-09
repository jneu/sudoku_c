#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROW_2_ROWZ(r) ((r) - 1)
#define COL_2_COLZ(c) ((c) - 1)
#define RCZ_2_X(rowz, colz) ((colz) + ((rowz) * 9))

typedef uint8_t value_t;
#define UNKNOWN_VALUE ((value_t) 0)

typedef struct def_cell
{
  value_t value;
  bool excluded[9];
} cell;

typedef struct def_grid
{
  cell cells[81];
  bool dirty;
} grid;

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

static void
grid_clear (grid * g)
{
  memset (g, 0, sizeof (grid));
}

static void
grid_set_exclusion_at_index (grid * g, int index, value_t value)
{
  value_t i;

  if (EXCLUDEDX (g, index, value))
    return;

  if (value == VALUEX (g, index))
    {
      fprintf (stderr,
               "%s: excluding cell value which is already set - index: %d value: %d\n",
               __FUNCTION__, index, (int) value);
      exit (EXIT_FAILURE);
    }

  EXCLUDEDX (g, index, value) = true;
  g->dirty = true;

  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDEDX (g, index, i))
        break;
    }

  if (9 == i)
    {
      fprintf (stderr, "%s: all values excluded for a cell - index: %d\n",
               __FUNCTION__, index);
      exit (EXIT_FAILURE);
    }
}

static void
grid_set_value_at_index (grid * g, int index, value_t value)
{
  value_t i;
  int j, k;
  int start;
  value_t cur_value = VALUEX (g, index);

  if (value == cur_value)
    return;

  if (UNKNOWN_VALUE != cur_value)
    {
      fprintf (stderr,
               "%s: cell value already set - index: %d old value: %d new value: %d\n",
               __FUNCTION__, index, (int) cur_value, (int) value);
      exit (EXIT_FAILURE);
    }

  if (EXCLUDEDX (g, index, value))
    {
      fprintf (stderr,
               "%s: value is excluded at this cell - index: %d value: %d\n",
               __FUNCTION__, index, (int) value);
      exit (EXIT_FAILURE);
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
        grid_set_exclusion_at_index (g, i, value);
    }

  start = index % 9;
  for (i = start; i < 81; i += 9)
    {
      if (i != index)
        grid_set_exclusion_at_index (g, i, value);
    }

  start = grid_box_start_index (index);
  for (j = 0; j < 3; j++)
    {
      for (k = 0; k < 3; k++)
        {
          if (start != index)
            grid_set_exclusion_at_index (g, start, value);

          start++;
        }

      start += 9 - 3;
    }
}

#if 0
static void
grid_set_value (grid * g, int rowz, int colz, value_t value)
{
  grid_set_value_at_index (g, RCZ_2_X (rowz, colz), value);
}
#endif

#if 0
static void
grid_add_given (grid * g, int row, int column, value_t value)
{
  if ((row < 1) || (row > 9))
    {
      fprintf (stderr,
               "%s: row must be between 1 and 9, inclusive - row: %d column: %d value: %d\n",
               __FUNCTION__, row, column, (int) value);
      exit (EXIT_FAILURE);
    }

  if ((column < 1) || (column > 9))
    {
      fprintf (stderr,
               "%s: column must be between 1 and 9, inclusive - row: %d column: %d value: %d\n",
               __FUNCTION__, row, column, (int) value);
      exit (EXIT_FAILURE);
    }

  if ((value < 1) || (value > 9))
    {
      fprintf (stderr,
               "%s: value must be between 1 and 9, inclusive - row: %d column: %d value: %d\n",
               __FUNCTION__, row, column, (int) value);
      exit (EXIT_FAILURE);
    }

  grid_set_value (g, ROW_2_ROWZ (row), COL_2_COLZ (column), value);
}
#endif

static void
grid_add_given_at_index (grid * g, int index, value_t value)
{
  if ((index < 0) || (index >= 81))
    {
      fprintf (stderr,
               "%s: index must be between 0 and 81, inclusive - index: %d value: %d\n",
               __FUNCTION__, index, (int) value);
      exit (EXIT_FAILURE);
    }

  if ((value < 1) || (value > 9))
    {
      fprintf (stderr,
               "%s: value must be between 1 and 9, inclusive - index: %d value: %d\n",
               __FUNCTION__, index, (int) value);
      exit (EXIT_FAILURE);
    }

  grid_set_value_at_index (g, index, value);
}

static void
grid_algo_only_one_available_in_cell (grid * g, int index)
{
  value_t i;
  value_t available;

  if (UNKNOWN_VALUE != VALUEX (g, index))
    return;

  available = UNKNOWN_VALUE;

  for (i = 1; i <= 9; i++)
    {
      if (!EXCLUDEDX (g, index, i))
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
      if (value == VALUEX (g, i))
        return;

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
      fprintf (stderr,
               "%s: value is not allowed in row - rowz: %d value: %d\n",
               __FUNCTION__, rowz, (int) value);
      exit (EXIT_FAILURE);
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
                      grid_set_exclusion_at_index (g, start, value);
                      start++;
                    }

                  start += 9 - 3;
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
      if (value == VALUEX (g, i))
        return;

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
      fprintf (stderr,
               "%s: value is not allowed in column - colz: %d value: %d\n",
               __FUNCTION__, colz, (int) value);
      exit (EXIT_FAILURE);
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

          start = grid_box_start_index (first);
          first_box_colz = first % 3;

          for (j = 0; j < 3; j++)
            {
              for (k = 0; k < 3; k++)
                {
                  if (k != first_box_colz)
                    grid_set_exclusion_at_index (g, start, value);

                  start++;
                }

              start += 9 - 3;
            }
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

int
main (void)
{
  grid g;

  grid_clear (&g);

#if 0
  grid_add_given (&g, 1, 3, 1);
  grid_add_given (&g, 1, 5, 6);
  grid_add_given (&g, 1, 6, 3);
  grid_add_given (&g, 1, 8, 7);
  grid_add_given (&g, 2, 2, 5);
  grid_add_given (&g, 2, 3, 8);
  grid_add_given (&g, 2, 6, 9);
  grid_add_given (&g, 2, 9, 6);
  grid_add_given (&g, 3, 1, 7);
  grid_add_given (&g, 4, 1, 5);
  grid_add_given (&g, 4, 3, 4);
  grid_add_given (&g, 4, 6, 2);
  grid_add_given (&g, 5, 1, 8);
  grid_add_given (&g, 5, 5, 7);
  grid_add_given (&g, 5, 9, 9);
  grid_add_given (&g, 6, 4, 4);
  grid_add_given (&g, 6, 7, 5);
  grid_add_given (&g, 6, 9, 7);
  grid_add_given (&g, 7, 9, 3);
  grid_add_given (&g, 8, 1, 2);
  grid_add_given (&g, 8, 4, 3);
  grid_add_given (&g, 8, 7, 8);
  grid_add_given (&g, 8, 8, 9);
  grid_add_given (&g, 9, 2, 8);
  grid_add_given (&g, 9, 4, 6);
  grid_add_given (&g, 9, 5, 1);
  grid_add_given (&g, 9, 7, 7);
#else
  {
    int i;
    const char puzzle[81] =
      ".8.....579.5...68......3.....8..65..32..1..46..49..2.....3......32...7.484.....6.";

    for (i = 0; i < 81; i++)
      {
        char c = puzzle[i];

        if ((c >= '1') && (c <= '9'))
          grid_add_given_at_index (&g, i, (value_t) (c - '1' + 1));
      }
  }
#endif

  while (g.dirty)
    {
      int i;

      do
        {
          g.dirty = false;

          for (i = 0; i < 81; i++)
            grid_algo_only_one_available_in_cell (&g, i);

          for (i = 0; i < 9; i++)
            {
              value_t v;

              for (v = 1; v <= 9; v++)
                {
                  grid_algo_need_one_or_bounded_in_rowz (&g, i, v);
                  grid_algo_need_one_or_bounded_in_colz (&g, i, v);
#if 0
                  /* @@@ is this ever needed? */
                  grid_algo_need_one_or_bounded_in_box (&g, i, v);
#endif
                }
            }
        }
      while (g.dirty);

      /* @@@ add in the pidgen hole algos */
    }

  grid_pretty_print (&g);

  return EXIT_SUCCESS;
}
