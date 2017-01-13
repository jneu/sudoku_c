#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "grid.h"

#define HISTORY_FILE "/Users/jneuheisel/.sudoku_history"

int
main (void)
{
  int rv;
  grid *g;
  bool done;

  rv = read_history (HISTORY_FILE);
  if (0 != rv)
    {
      if (ENOENT != rv)
        printf ("warning: failed to read history file (%s) - %s\n", HISTORY_FILE, strerror (rv));
    }

  grid_create (&g);

  done = false;
  do
    {
      char *line;
      bool run_solve = false;

      line = readline ("> ");
      if (NULL == line)
        {
          printf ("\n");

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

  rv = write_history (HISTORY_FILE);
  if (0 != rv)
    {
      printf ("warning: failed to write history file (%s) - %s\n", HISTORY_FILE, strerror (rv));
    }

#if USE_VALGRIND
  grid_destroy (g);
  rl_clear_history ();
#endif

  return EXIT_SUCCESS;
}
