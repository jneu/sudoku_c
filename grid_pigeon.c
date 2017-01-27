#include <string.h>

#include "grid_internal.h"

typedef struct def_block
{
  int items[9];
  int num_items;
} block;

typedef struct def_pigeon_context pigeon_context;
typedef void (*pigeon_callback) (pigeon_context *);

struct def_pigeon_context
{
  const block *b_orig;
  int subset_card;

  int offset;
  grid *g;

  block b_working;
  bool excluded_working[9];

  pigeon_callback handle_pigeon;
};

static void
enumerate_subsets_recurse (pigeon_context * context, int depth, int from)
{
  int i;
  int start_num_items;
  const block *b_orig = context->b_orig;
  block *b_working = &context->b_working;

  start_num_items = b_working->num_items++;
  depth--;

  for (i = from; i < b_orig->num_items - depth; i++)
    {
      int j, k;
      int num_not_fully_excluded;
      cell *c;
      bool *p;

      b_working->items[start_num_items] = b_orig->items[i];

      if (0 != depth)
        {
          enumerate_subsets_recurse (context, depth, i + 1);
          continue;
        }

      c = context->g->cells;
      p = context->excluded_working;

      memcpy (p, c[b_working->items[0]].excluded, 9 * sizeof (bool));

      for (j = 1; j < context->subset_card; j++)
        {
          const bool *p_orig = c[b_working->items[j]].excluded;

          for (k = 0; k < 9; k++)
            p[k] &= p_orig[k];
        }

      num_not_fully_excluded = 0;
      for (j = 0; j < 9; j++)
        {
          if (!p[j])
            num_not_fully_excluded++;
        }

      if (num_not_fully_excluded == context->subset_card)
        context->handle_pigeon (context);
    }

  b_working->num_items = start_num_items;
}

static void
enumerate_subsets (grid * g, int offset, int subset_card, const block * b_orig, pigeon_callback handle_pigeon)
{
  pigeon_context context;

  context.b_orig = b_orig;
  context.subset_card = subset_card;
  context.offset = offset;
  context.g = g;
  context.b_working.num_items = 0;
  context.handle_pigeon = handle_pigeon;

  enumerate_subsets_recurse (&context, subset_card, 0);
}

static void
pigeon_clear_row (pigeon_context * context)
{
  int i, j;
  int subset_card = context->subset_card;
  int rowz = context->offset;
  grid *g = context->g;
  block *b_working = &context->b_working;
  bool *excluded_working = context->excluded_working;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      bool use_it;

      if (UNKNOWN_VALUE != VALUE (g, i))
        continue;

      use_it = true;

      for (j = 0; j < subset_card; j++)
        {
          if (i == b_working->items[j])
            {
              use_it = false;
              break;
            }
        }

      if (!use_it)
        continue;

      for (j = 0; j < 9; j++)
        {
          if (!excluded_working[j])
            grid_set_exclusion_at_index (g, i, (value_t) (j + 1));
        }
    }
}

void
grid_algo_pigeon_vacant_in_rowz (grid * g, int rowz)
{
  int i;
  int subset_card;
  block vacant;

  vacant.num_items = 0;

  for (i = rowz * 9; i < (rowz + 1) * 9; i++)
    {
      if (UNKNOWN_VALUE == VALUE (g, i))
        vacant.items[vacant.num_items++] = i;
    }

  for (subset_card = 2; subset_card <= (vacant.num_items - 2); subset_card++)
    enumerate_subsets (g, rowz, subset_card, &vacant, pigeon_clear_row);
}

static void
pigeon_clear_column (pigeon_context * context)
{
  int i, j;
  int subset_card = context->subset_card;
  int colz = context->offset;
  grid *g = context->g;
  block *b_working = &context->b_working;
  bool *excluded_working = context->excluded_working;

  for (i = colz; i < 81; i += 9)
    {
      bool use_it;

      if (UNKNOWN_VALUE != VALUE (g, i))
        continue;

      use_it = true;

      for (j = 0; j < subset_card; j++)
        {
          if (i == b_working->items[j])
            {
              use_it = false;
              break;
            }
        }

      if (!use_it)
        continue;

      for (j = 0; j < 9; j++)
        {
          if (!excluded_working[j])
            grid_set_exclusion_at_index (g, i, (value_t) (j + 1));
        }
    }
}

void
grid_algo_pigeon_vacant_in_colz (grid * g, int colz)
{
  int i;
  int subset_card;
  block vacant;

  vacant.num_items = 0;

  for (i = colz; i < 81; i += 9)
    {
      if (UNKNOWN_VALUE == VALUE (g, i))
        vacant.items[vacant.num_items++] = i;
    }

  for (subset_card = 2; subset_card <= (vacant.num_items - 2); subset_card++)
    enumerate_subsets (g, colz, subset_card, &vacant, pigeon_clear_column);
}

static void
pigeon_clear_box (pigeon_context * context)
{
  int i, j, k;
  int start;
  int subset_card = context->subset_card;
  int box = context->offset;
  grid *g = context->g;
  block *b_working = &context->b_working;
  bool *excluded_working = context->excluded_working;

  start = BOX_START_FROM_WHICH (box);

  for (i = 0; i < 3; i++, start += 6)
    {
      for (j = 0; j < 3; j++, start++)
        {
          bool use_it;

          if (UNKNOWN_VALUE != VALUE (g, start))
            continue;

          use_it = true;

          for (k = 0; k < subset_card; k++)
            {
              if (start == b_working->items[k])
                {
                  use_it = false;
                  break;
                }
            }

          if (!use_it)
            continue;

          for (k = 0; k < 9; k++)
            {
              if (!excluded_working[k])
                grid_set_exclusion_at_index (g, start, (value_t) (k + 1));
            }
        }
    }
}

void
grid_algo_pigeon_vacant_in_box (grid * g, int box)
{
  int i, j;
  int start;
  int subset_card;
  block vacant;

  vacant.num_items = 0;
  start = BOX_START_FROM_WHICH (box);

  for (i = 0; i < 3; i++, start += 6)
    {
      for (j = 0; j < 3; j++, start++)
        {
          if (UNKNOWN_VALUE == VALUE (g, start))
            vacant.items[vacant.num_items++] = start;
        }
    }

  for (subset_card = 2; subset_card <= (vacant.num_items - 2); subset_card++)
    enumerate_subsets (g, box, subset_card, &vacant, pigeon_clear_box);
}
