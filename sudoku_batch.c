#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

int
main (void)
{
  grid *g;
  char line[82];
  int num_solved = 0;

  grid_create (&g);

  for (;;)
    {
      int i;
      size_t rv;
      bool success;

      rv = fread (line, 1, 82, stdin);
      if (0 == rv)
        break;

      if ((82 != rv) || ('\n' != line[81]))
        {
          fprintf (stderr, "error reading the puzzles\n");
          exit (EXIT_FAILURE);
        }
      line[81] = '\0';

      grid_clear (g);

      for (i = 0; i < 81; i++)
        {
          char c = line[i];

          if ((c >= '1') && (c <= '9'))
            {
              success = grid_add_given_value_at_index (g, i, (value_t) (c - '1' + 1));

              if (!success)
                exit (EXIT_FAILURE);
            }
        }

      success = grid_solve (g);

      if (!success)
        exit (EXIT_FAILURE);

      if (grid_is_solved (g))
        num_solved++;
      else
        {
          printf ("%s\n", line);
          grid_pretty_print (g);
        }
    }

  printf ("Number solved: %d\n", num_solved);

#if USE_VALGRIND
  grid_destroy (g);
#endif

  return EXIT_SUCCESS;
}
