#include "grid_internal.h"

void
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

void
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

void
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

void
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
