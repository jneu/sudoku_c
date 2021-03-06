#include <stdio.h>
#include <stdlib.h>

#include "grid.h"

int
main (int argc, char *argv[])
{
  grid *g;
  char line[82];
  int num_solved = 0;
  FILE *fp = stdin;

  if (argc > 1)
    {
      fp = fopen (argv[1], "rb");
      if (NULL == fp)
        {
          perror ("failed to open input file\n");
          exit (EXIT_FAILURE);
        }
    }

  grid_create (&g);

  for (;;)
    {
      int i;
      size_t rv;

      rv = fread (line, 1, 82, fp);
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
            grid_add_given_value_at_index (g, i, (value_t) (c - '1' + 1));
        }

      grid_solve (g);

      if (!grid_is_consistent (g))
        {
          printf ("inconsistent\n");
          exit (EXIT_FAILURE);
        }

      if (grid_is_solved (g))
        {
          num_solved++;
        }
      else
        {
          printf ("%s\n", line);
          grid_pretty_print (g);
        }
    }

  printf ("Number solved: %d\n", num_solved);

  fclose (fp);

#if USE_VALGRIND
  grid_destroy (g);
#endif

  return EXIT_SUCCESS;
}
