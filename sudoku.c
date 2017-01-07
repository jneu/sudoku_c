#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t value_t;
#define UNKNOWN_VALUE ((value_t) 0)

typedef struct def_cell
{
  value_t value;
  bool excluded[9];
} cell;

typedef struct def_grid
{
  cell cells[9 * 9];
} grid;

static void
grid_clear (grid * g)
{
  memset (g, 0, sizeof (grid));
}

static void
grid_add_given (grid * g, int row, int column, int value)
{
  if ((row < 1) || (row > 9))
    {
      fprintf (stderr,
               "%s: row must be between 1 and 9, inclusive - row: %d column: %d value: %d\n",
               __FUNCTION__, row, column, value);
      exit (EXIT_FAILURE);
    }

  if ((column < 1) || (column > 9))
    {
      fprintf (stderr,
               "%s: column must be between 1 and 9, inclusive - row: %d column: %d value: %d\n",
               __FUNCTION__, row, column, value);
      exit (EXIT_FAILURE);
    }

  if ((value < 1) || (value > 9))
    {
      fprintf (stderr,
               "%s: value must be between 1 and 9, inclusive - row: %d column: %d value: %d\n",
               __FUNCTION__, row, column, value);
      exit (EXIT_FAILURE);
    }

  g->cells[(column - 1) + (row - 1) * 9].value = value;
}

void
grid_pretty_print (const grid * g)
{
  int i, j, k, m;

  for (i = 0; i < 3; i++)
    {
      if (0 != i)
        printf ("-------------\n");

      for (j = 0; j < 3; j++)
        {
          for (k = 0; k < 3; k++)
            {
              if (0 != k)
                printf ("|");

              for (m = 0; m < 3; m++)
                {
                  value_t v;

                  v = g->cells[(m + k * 3) + (j + i * 3) * 9].value;

                  if (UNKNOWN_VALUE == v)
                    printf (" ");
                  else
                    printf ("%d", (int) v);
                }
            }

          printf ("\n");
        }
    }
}

int
main (void)
{
  grid g;

  grid_clear (&g);

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

  grid_pretty_print (&g);

  return EXIT_SUCCESS;
}
