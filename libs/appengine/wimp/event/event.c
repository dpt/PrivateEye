/* --------------------------------------------------------------------------
 *    Name: event.c
 * Purpose: Event library
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "oslib/os.h"
#include "oslib/osmodule.h"
#include "oslib/wimp.h"

#include "appengine/wimp/event.h"

#include "event-wimp.h"
#include "event-message.h"

static wimp_poll_flags event_mask     = 0;
static void           *event_pollword;
static os_t            event_interval = 0;

/* ----------------------------------------------------------------------- */

void event_initialise(void)
{
  /* allocate space for a pollword */
  xosmodule_alloc(4, &event_pollword);
  event_zero_pollword();

  event_initialise_message();
}

void event_finalise(void)
{
  event_finalise_wimp();
  event_finalise_message();

  xosmodule_free(event_pollword);
}

/* ----------------------------------------------------------------------- */

void event_set_mask(wimp_poll_flags mask)
{
  event_mask = mask;
}

void event_set_interval(os_t t)
{
  event_interval = t;
}

/* ----------------------------------------------------------------------- */

void event_zero_pollword(void)
{
  *((int *) event_pollword) = 0;
}

void *event_get_pollword(void)
{
  return event_pollword;
}

/* ----------------------------------------------------------------------- */

wimp_event_no event_poll(wimp_block *block)
{
  wimp_poll_flags poll_flags;
  wimp_event_no   event_no;

  poll_flags = event_mask | event_wimp_get_mask();

  if (poll_flags & wimp_MASK_NULL)
  {
    event_no = wimp_poll(poll_flags, block, event_pollword);
  }
  else
  {
    os_t t;

    t = os_read_monotonic_time() + event_interval;

    event_no = wimp_poll_idle(event_mask, block, t, event_pollword);
  }

  (void) event_dispatch_wimp(event_no, block);

  return event_no;
}
