/* --------------------------------------------------------------------------
 *    Name: redraw.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

/* ----------------------------------------------------------------------- */

static event_wimp_handler tag_cloud_redraw_event_null_reason_code;

/* ----------------------------------------------------------------------- */

static void claim_nulls(int reg, tag_cloud *tc)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_NULL_REASON_CODE, tag_cloud_redraw_event_null_reason_code },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            tc->main_w,
                            event_ANY_ICON,
                            tc);
}

static int tag_cloud_redraw_event_null_reason_code(wimp_event_no event_no,
                                                   wimp_block   *block,
                                                   void         *handle)
{
  tag_cloud *tc;
  wimp_window_state state;

  NOT_USED(event_no);
  NOT_USED(block);

  tc = handle;

  state.w = tc->main_w;
  wimp_get_window_state(&state);

  if (state.flags & wimp_WINDOW_OPEN)
  {
    tag_cloud_layout(tc, tc->layout.width /* no change */);
    wimp_force_redraw(tc->main_w, 0, -16384, 16384, 0);
  }

  claim_nulls(0, tc);

  return event_HANDLED;
}

void tag_cloud_schedule_redraw(tag_cloud *tc)
{
  /* This will ignore already-registered handlers. */
  claim_nulls(1, tc);
}
