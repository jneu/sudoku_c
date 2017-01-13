#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "grid.h"

int
main (void)
{
  grid *g;
  bool done;

  grid_create (&g);

  done = false;
  do
    {
      char *line;
      bool run_solve = false;

      line = readline ("> ");
      if (NULL == line)
        {
          done = true;
          continue;
        }

      if ('\0' == line[0])
        {
          run_solve = true;
        }
      else
        {
          int row, column, value;

          add_history (line);

          if (3 == sscanf (line, "s %d %d %d", &row, &column, &value))
            {
              run_solve = grid_add_given_value (g, row, column, value);
            }
          else if (3 == sscanf (line, "x %d %d %d", &row, &column, &value))
            {
              run_solve = grid_add_given_exclusion (g, row, column, value);
            }
          else if (('g' == line[0]) && (' ' == line[1]) && (83 == strlen (line)))
            {
              int i;

              run_solve = true;

              grid_clear (g);

              for (i = 0; i < 81; i++)
                {
                  char c = line[i + 2];

                  if ((c >= '1') && (c <= '9'))
                    run_solve &= grid_add_given_value_at_index (g, i, (value_t) (c - '1' + 1));
                }
            }
          else
            {
              printf ("syntax error\n");
            }
        }

      free (line);

      if (run_solve)
        {
          if (grid_solve (g))
            grid_pretty_print (g);
        }
    }
  while (!done);

  grid_destroy (g);
  rl_clear_history ();

  return EXIT_SUCCESS;
}
