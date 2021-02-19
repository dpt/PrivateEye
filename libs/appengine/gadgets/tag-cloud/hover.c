/* --------------------------------------------------------------------------
 *    Name: hover.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "datastruct/atom.h"

#include "appengine/wimp/pointer.h"
#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

static const char ptr_tag[]   = "ptr_tag";
static const char ptr_detag[] = "ptr_detag";

static void wipe_underline(tag_cloud *tc, int index)
{
  const os_box *b;

  if (index < 0)
    return;

  b = &tc->layout.boxes.boxes[index];

  wimp_force_redraw(tc->main_w,
                    b->x0,
                    b->y0,
                    b->x1,
                    b->y0 + (b->y1 - b->y0) / 4);
}

void tag_cloud_hover_init(tag_cloud *tc)
{
  /* layout calls this to reset the hover, when tag data has changed */
  tc->hover.index      = -1;
  tc->hover.last_index = -1;
}

void tag_cloud_hover(tag_cloud *tc, int x, int y)
{
  atom_t last_index;
  int    index;

  last_index = tc->hover.last_index;

  if (tc->flags & tag_cloud_FLAG_SHADED)
    index = -1; /* hover is disabled when we're shaded */
  else
    index = tag_cloud_hit(tc, x, y);

  if (index == last_index)
    return;

  if (index >= 0)
  {
    int         highlight;
    const char *ptrname;

    highlight = tag_cloud_is_highlighted(tc, index);
    ptrname   = highlight ? ptr_detag : ptr_tag;

    if (tc->hover.pointer_shape_name != ptrname)
    {
      set_pointer_shape(ptrname, 0, 0);
      tc->hover.pointer_shape_name = ptrname;
    }
  }
  else
  {
    tag_cloud_restore_pointer_shape(tc);
  }

  wipe_underline(tc, tc->hover.last_index); /* erase previous */

  tc->hover.index = index;

  tc->flags |= tag_cloud_FLAG_NEW_HOVER;

  tag_cloud_layout(tc, tc->layout.width /* no change */);

  wipe_underline(tc, index); /* draw new */

  tc->hover.last_index = index;
}

void tag_cloud_hover_toggle(tag_cloud *tc)
{
  const char *ptrname;

  ptrname = tc->hover.pointer_shape_name;

  if (ptrname == ptr_tag)
    ptrname = ptr_detag;
  else if (ptrname == ptr_detag)
    ptrname = ptr_tag;
  else
    return;

  set_pointer_shape(ptrname, 0, 0);
  tc->hover.pointer_shape_name = ptrname;
}

void tag_cloud_restore_pointer_shape(tag_cloud *tc)
{
  if (tc->hover.pointer_shape_name)
  {
    restore_pointer_shape();
    tc->hover.pointer_shape_name = NULL;
  }
}
