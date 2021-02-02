/* --------------------------------------------------------------------------
 *    Name: treeview.c
 * Purpose: Tree view
 * ----------------------------------------------------------------------- */

/* Each node is drawn on its own line (nodes never share a line). */

#include <assert.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/osspriteop.h"
#include "oslib/wimp.h"

#include "datastruct/bitvec.h"
#include "datastruct/ntree.h"
#include "geom/box.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/text/txtfmt.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/treeview.h"

/* ----------------------------------------------------------------------- */

/* an estimate of the number of nodes we'll be drawing
 * (used for sizing bitvecs) */
#define NNODES_ESTIMATE 128

/* default text formatting width */
#define DEFAULT_TEXT_WIDTH 40

/* width and height of a connector, height of text objects */
#define DEFAULT_LINE_HEIGHT 44

/* whether to skip the root node */
#define SKIP_ROOT 1

/* ----------------------------------------------------------------------- */

typedef unsigned int connector_flags;

#define ABOVE    (1u << 4)
#define BELOW    (1u << 3)
#define RIGHT    (1u << 2)
#define OPENABLE (1u << 1)
#define OPEN     (1u << 0) /* only makes sense if connector is OPENABLE */

/* ----------------------------------------------------------------------- */

typedef result_t (treeview_callback)(treeview_t      *tr,
                                  int              depth,
                                  int              x,
                                  int              y,
                                  ntree_t         *t,
                                  connector_flags  flags,
                                  int              index,
                                  txtfmt_t        *tx,
                                  int              height,
                                  void            *opaque);

struct treeview_t
{
  ntree_t             *tree;

  bitvec_t            *openable;
  bitvec_t            *open;
  bitvec_t            *multiline;

  int                  text_width;

  wimp_colour          bgcolour;    /* background of highlighted nodes */

  int                  line_height; /* OS units */

  struct
  {
    int                x,y;
    int                index;
    ntree_t           *next;
    treeview_callback *cb;
    void              *cbarg;
  }
  walk;
};

/* ----------------------------------------------------------------------- */

static unsigned int treeview_refcount = 0;

result_t treeview_init(void)
{
  treeview_refcount++;

  return result_OK;
}

void treeview_fin(void)
{
  if (treeview_refcount == 0)
    return;

  --treeview_refcount;
}

/* ----------------------------------------------------------------------- */

static result_t discard_tree_callback(ntree_t *t, void *opaque)
{
  NOT_USED(opaque);

  txtfmt_destroy(ntree_get_data(t));

  return result_OK;
}

static result_t discard_tree(ntree_t *t)
{
  if (!t)
    return result_OK;

  (void) ntree_walk(t,
                    ntree_WALK_POST_ORDER | ntree_WALK_ALL,
                    0,
                    discard_tree_callback,
                    NULL);

  ntree_delete(t);

  return result_OK;
}

/* ----------------------------------------------------------------------- */

result_t treeview_create(treeview_t **tr)
{
  treeview_t *newtr;

  newtr = calloc(1, sizeof(*newtr));
  if (newtr == NULL)
    return result_OOM;

  newtr->openable  = bitvec_create(NNODES_ESTIMATE);
  newtr->open      = bitvec_create(NNODES_ESTIMATE);
  newtr->multiline = bitvec_create(NNODES_ESTIMATE);
  if (newtr->openable  == NULL ||
      newtr->open      == NULL ||
      newtr->multiline == NULL)
    goto OOM;

  newtr->text_width  = DEFAULT_TEXT_WIDTH;
  newtr->bgcolour    = wimp_COLOUR_WHITE; /* default */
  newtr->line_height = DEFAULT_LINE_HEIGHT;

  *tr = newtr;

  return result_OK;


OOM:

  treeview_destroy(newtr);

  return result_OOM;
}

void treeview_destroy(treeview_t *tr)
{
  bitvec_destroy(tr->multiline);
  bitvec_destroy(tr->open);
  bitvec_destroy(tr->openable);

  discard_tree(tr->tree);

  free(tr);
}

/* ----------------------------------------------------------------------- */

static result_t treeview_walk_node(ntree_t *t, void *opaque)
{
  result_t            err;
  treeview_t      *tr = opaque;
  int              x,y;
  connector_flags  flags;
  int              stop = 0;
  int              depth;
  txtfmt_t        *tx;
  int              height;

  x = tr->walk.x;
  y = tr->walk.y - tr->line_height;

  tr->walk.index++; /* index of node (0..N-1) */

  /* retrieve the flags from bitvecs */

  flags = 0;

  if (bitvec_get(tr->openable, tr->walk.index))
  {
    flags |= OPENABLE;
    if (bitvec_get(tr->open, tr->walk.index))
      flags |= OPEN;
  }

  /* if 'next' is set then skip nodes until we hit the one we want */

  if (tr->walk.next)
  {
    if (tr->walk.next != t)
      return result_OK; /* skip */

    tr->walk.next = NULL; /* stop skipping */
  }

  /* skip nodes whose parent is collapsed */

  if ((flags & OPENABLE) && (flags & OPEN) == 0)
  {
    ntree_t *next;

    /* we want to process /this/ node and then skip all intermediate ones
     * until the next sibling (if any) */

    next = ntree_next_sibling(t);

    /* if there's no next sibling, then we need to look further up/ahead in
     * the tree */

    if (next == NULL)
    {
      ntree_t *cur;
      ntree_t *par;

      for (cur = t; ; cur = par)
      {
        /* fetch the parent (which we've already seen) */

        par = ntree_parent(cur);
        if (par == NULL)
        {
          stop = 1;
          break; /* no parent? must be at the root, so give up */
        }

        next = ntree_next_sibling(par);
        if (next)
          break;
      }
    }

    tr->walk.next = next;
  }

  /* it might be cheaper for ntree_walk to pass the depth into us than to
   * have to find it ourselves here */

  depth = ntree_depth(t) - 1;

#if SKIP_ROOT
  if (depth == 0)
    return result_OK;

  depth--;
#endif

  tx = ntree_get_data(t);

  /* need to know how high the text will be */

  height = txtfmt_get_height(tx);

  /* call the callback */

  if (tr->walk.cb)
  {
    err = tr->walk.cb(tr,
                      depth,
                      x,
                      y,
                      t,
                      flags,
                      tr->walk.index,
                      tx,
                      height,
                      tr->walk.cbarg);
    if (err)
      return err;
  }

  /* calculate new y */

  if (bitvec_get(tr->multiline, tr->walk.index))
    y -= tr->line_height * (height - 1); /* -1 to adjust for initial step */

  tr->walk.x = x;
  tr->walk.y = y;

  /* if the walk needs to stop now, then we raise a non-terminal error to
   * make ntree_walk stop */

  return (stop) ? result_TREEVIEW_STOP_WALK : result_OK;
}

static result_t treeview_walk(treeview_t        *tr,
                           treeview_callback *cb,
                           void              *cbarg)
{
  result_t err;

  tr->walk.x     = 0;
  tr->walk.y     = 0;
  tr->walk.index = -1;
  tr->walk.next  = NULL;
  tr->walk.cb    = cb;
  tr->walk.cbarg = cbarg;

  err = ntree_walk(tr->tree,
                   ntree_WALK_PRE_ORDER | ntree_WALK_ALL,
                   0,
                   treeview_walk_node,
                   tr);

  if (err == result_TREEVIEW_STOP_WALK)
    err = result_OK;

  return err;
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  connector_flags key;
  const char     *name;
}
connector_info;

static const connector_info connectors[] =
{
  {                 RIGHT                   , "tr-00c00" },
  {                 RIGHT | OPENABLE        , "tr-00ct0" },
  {                 RIGHT | OPENABLE | OPEN , "tr-00cto" },
  {         BELOW | RIGHT                   , "tr-0bc00" },
  {         BELOW | RIGHT | OPENABLE        , "tr-0bct0" },
  {         BELOW | RIGHT | OPENABLE | OPEN , "tr-0bcto" },
  { ABOVE |         RIGHT                   , "tr-a0c00" },
  { ABOVE |         RIGHT | OPENABLE        , "tr-a0ct0" },
  { ABOVE |         RIGHT | OPENABLE | OPEN , "tr-a0cto" },
  { ABOVE | BELOW                           , "tr-ab000" }, /* this is VBAR */
  { ABOVE | BELOW | RIGHT                   , "tr-abc00" },
  { ABOVE | BELOW | RIGHT | OPENABLE        , "tr-abct0" },
  { ABOVE | BELOW | RIGHT | OPENABLE | OPEN , "tr-abcto" },
};

#define VBAR 9

static int flags_to_connector(connector_flags flags)
{
  int i;

  i = bsearch_uint(&connectors[0].key,
                    NELEMS(connectors),
                    sizeof(connectors[0]),
                    flags);
  if (i >= 0)
    return i;

  assert("Impossible flags" == NULL);

  return -1;
}

static result_t plot_connector(int index, int x, int y, int line_height)
{
  wimp_icon icon;

  icon.extent.x0 = x;
  icon.extent.y0 = y;
  icon.extent.x1 = x + line_height;
  icon.extent.y1 = y + line_height;
  icon.flags = wimp_ICON_SPRITE | wimp_ICON_HCENTRED | wimp_ICON_VCENTRED;
  strncpy(icon.data.sprite, connectors[index].name, 12);

  wimp_plot_icon(&icon);

  return result_OK;
}

/* ----------------------------------------------------------------------- */

typedef struct treeview_draw_data
{
  unsigned int stack; /* 32-deep stack of bits for drawing connectors */
}
treeview_draw_data;

static result_t draw_connectors(treeview_t         *tr,
                             ntree_t            *t,
                             treeview_draw_data *draw,
                             int                 depth,
                             int                 x,
                             int                 y,
                             connector_flags     flags,
                             int                 index)
{
  int          i;
  unsigned int stack;
  int          cx;

  /* plot connectors */

  /* draw->stack may have bits set from previous depths, so be careful to
   * range check */

  stack = draw->stack;
  cx    = x;
  for (i = 0; i < depth; i++)
  {
    if (stack & (1u << i))
    {
      plot_connector(VBAR, cx, y, tr->line_height);
      stack &= stack - 1; /* clear LSB */
      if (!stack)
        break;
    }
    cx += tr->line_height;
  }

  /* line above */

#if SKIP_ROOT
  if (index != 1)
#endif
    if (ntree_prev_sibling(t) || ntree_parent(t))
      flags |= ABOVE;

  if (ntree_next_sibling(t))
    flags |= BELOW;

  /* anything with a child or data must have a right connection */

  flags |= RIGHT;

  plot_connector(flags_to_connector(flags),
                 x + depth * tr->line_height,
                 y,
                 tr->line_height);

  /* maintain a 'stack' of lines we're drawing */

  if (flags & BELOW)
    draw->stack |= 1u << depth; /* 'stack' it */
  else
    draw->stack &= ~(1u << depth); /* clear it */

  return result_OK;
}

static result_t draw_walk(treeview_t     *tr,
                       int             depth,
                       int             x,
                       int             y,
                       ntree_t        *t,
                       connector_flags flags,
                       int             index,
                       txtfmt_t       *tx,
                       int             height,
                       void           *opaque)
{
  result_t               err;
  treeview_draw_data *draw = opaque;
  int                 line_height;
  wimp_colour         bgcolour;
  int                 x0;

  /* plot initial connectors */

  err = draw_connectors(tr, t, draw, depth, x, y, flags, index);
  if (err)
    goto Failure;

  line_height = tr->line_height;

  if (bitvec_get(tr->multiline, index))
  {
    int i;
    int hy;

    /* plot remainder of connectors */

    for (i = 0; i < depth + 1; i++)
      if (draw->stack & (1u << i))
        for (hy = y - 1 * line_height;
             hy != y - height * line_height;
             hy -= line_height)
          plot_connector(VBAR, x + i * line_height, hy, line_height);
  }

  /* select background colour */

  bgcolour = ntree_first_child(t) ? wimp_COLOUR_TRANSPARENT :
                                    tr->bgcolour;

  /* plot text */

  x0 = x + (depth + 1) * line_height;
  err = txtfmt_paint(tx, x0, y, bgcolour);
  if (err)
    goto Failure;

  return result_OK;


Failure:

  return err;
}

result_t treeview_draw(treeview_t *tr)
{
  result_t              err;
  treeview_draw_data data;

  data.stack = 0;

  err = treeview_walk(tr, draw_walk, &data);

  return err;
}

/* ----------------------------------------------------------------------- */

typedef struct treeview_click_data
{
  int x,y;
}
treeview_click_data;

static result_t click_walk(treeview_t    *tr,
                       int             depth,
                       int             x,
                       int             y,
                       ntree_t        *t,
                       connector_flags flags,
                       int             index,
                       txtfmt_t       *tx,
                       int             height,
                       void           *opaque)
{
  treeview_click_data *data = opaque;
  os_box               box;

  NOT_USED(tr);
  NOT_USED(t);
  NOT_USED(flags);
  NOT_USED(index);
  NOT_USED(tx);
  NOT_USED(height);

  /* hit detect */

  box.x0 = x + depth * tr->line_height;
  box.y0 = y;
  box.x1 = box.x0 + tr->line_height;
  box.y1 = box.y0 + tr->line_height;

  if (box_contains_point(&box, data->x, data->y))
    /* exiting here means that tr->walk.x and y are valid */
    return result_TREEVIEW_FOUND;
  else
    return result_OK;
}

result_t treeview_click(treeview_t *tr, int x, int y, int *redraw_y)
{
  result_t               err;
  treeview_click_data click;

  click.x = x;
  click.y = y;

  err = treeview_walk(tr, click_walk, &click);

  if (err == result_TREEVIEW_FOUND) /* found it */
  {
    /* tr->walk.y, tr->walk.index updated */

    /* the user may have clicked in the position of a collapse control,
     * even if one was not present, so check */

    if (bitvec_get(tr->openable, tr->walk.index))
    {
      bitvec_toggle(tr->open, tr->walk.index);

      *redraw_y = tr->walk.y;
    }
    else
    {
      *redraw_y = +1; /* no update */
    }

    err = result_OK;
  }
  else if (err == result_OK)
  {
    *redraw_y = +1; /* no update */
  }

  return err;
}

/* ----------------------------------------------------------------------- */

static result_t string_to_txtfmt(void *data, void *opaque, void **newdata)
{
  result_t       err;
  treeview_t *tr = opaque;
  txtfmt_t   *tx;

  err = txtfmt_create(data, &tx);
  if (err)
    return err;

  txtfmt_set_line_height(tx, tr->line_height);

  *newdata = tx;

  return err;
}

result_t treeview_set_tree(treeview_t *tr, ntree_t *tree)
{
  result_t err;

  if (tr->tree)
  {
    discard_tree(tr->tree);
    tr->tree = NULL;
  }

  /* clone the tree, converting its strings into txtfmts */

  err = ntree_copy(tree, string_to_txtfmt, tr, &tr->tree);
  if (err)
    return err;

  return result_OK;
}

/* ----------------------------------------------------------------------- */

static result_t make_collapsible_walk(ntree_t *t, void *opaque)
{
  result_t       err;
  treeview_t *tr = opaque;
  int         i;
  txtfmt_t   *tx;

  i = tr->walk.index + 1; /* index of node (0..N-1) */

  if (ntree_first_child(t))
  {
    err = bitvec_set(tr->openable, i);
    if (err)
      return err;

    err = bitvec_set(tr->open, i);
    if (err)
      return err;
  }

  // this bit might be temporary

  tx = ntree_get_data(t);

  // always wrap to ensure that \ns in the input are handled
  err = txtfmt_wrap(tx, tr->text_width);
  if (err)
    return err;

  if (txtfmt_get_height(tx) > 1)
  {
    err = bitvec_set(tr->multiline, i);
    if (err)
      return err;
  }

  tr->walk.index = i;

  return result_OK;
}

// i get the feeling that this might need to be generalised into a 'format the tree' call
result_t treeview_make_collapsible(treeview_t *tr)
{
  result_t err;

  tr->walk.index = -1;

  err = ntree_walk(tr->tree,
                   ntree_WALK_PRE_ORDER | ntree_WALK_ALL,
                   0,
                   make_collapsible_walk,
                   tr);

  return err;
}

/* ----------------------------------------------------------------------- */

void treeview_set_text_width(treeview_t *tr, int width)
{
  tr->text_width = width;
}

void treeview_set_line_height(treeview_t *tr, int line_height)
{
  tr->line_height = line_height;
}

/* ----------------------------------------------------------------------- */

typedef struct treeview_get_dimensions_data
{
  int width;
}
treeview_get_dimensions_data;

static result_t get_dimensions_walk(treeview_t     *tr,
                                 int             depth,
                                 int             x,
                                 int             y,
                                 ntree_t        *t,
                                 connector_flags flags,
                                 int             index,
                                 txtfmt_t       *tx,
                                 int             height,
                                 void           *opaque)
{
  treeview_get_dimensions_data *data = opaque;
  int                           width;

  NOT_USED(tr);
  NOT_USED(y);
  NOT_USED(t);
  NOT_USED(flags);
  NOT_USED(index);
  NOT_USED(height);

  width = x;
  width += (depth + 1) * tr->line_height;
  width += 16 * txtfmt_get_wrapped_width(tx);
  width += 16; /* add another character's worth of space to allow for Wimp
                  text padding */
  if (width > data->width)
    data->width = width;

  return result_OK;
}

result_t treeview_get_dimensions(treeview_t *tr, int *width, int *height)
{
  result_t                         err;
  treeview_get_dimensions_data data;

  data.width = 0;

  err = treeview_walk(tr, get_dimensions_walk, &data);
  if (err)
    return err;

  *width  = data.width;
  *height = tr->walk.y;

  return result_OK;
}

/* ----------------------------------------------------------------------- */

void treeview_set_highlight_background(treeview_t *tr, wimp_colour bgcolour)
{
  tr->bgcolour = bgcolour;
}

/* ----------------------------------------------------------------------- */

void treeview_mark_all(treeview_t *tr, treeview_mark mark)
{
  switch (mark)
  {
  case treeview_mark_COLLAPSE:
    bitvec_clear_all(tr->open);
    break;

  case treeview_mark_EXPAND:
    bitvec_set_all(tr->open);
    break;
  }
}
