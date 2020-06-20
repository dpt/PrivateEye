/* --------------------------------------------------------------------------
 *    Name: event.c
 * Purpose: Event library
 * ----------------------------------------------------------------------- */

#include <limits.h>

#include "fortify/fortify.h"

#include "oslib/os.h"
#include "oslib/osmodule.h"
#include "oslib/wimp.h"

#include "appengine/wimp/event.h"

#include "event-wimp.h"
#include "event-message.h"

static struct
{
  wimp_poll_flags mask;
  os_t            earliest;
  void           *pollword;
}
LOCALS;

/* ----------------------------------------------------------------------- */

void event_initialise(void)
{
  /* allocate space for a pollword */
  xosmodule_alloc(4, &LOCALS.pollword);
  event_zero_pollword();

  event_initialise_message();
}

void event_finalise(void)
{
  event_finalise_wimp();
  event_finalise_message();

  xosmodule_free(LOCALS.pollword);
}

/* ----------------------------------------------------------------------- */

// replace this with one counter per event?
void event_set_mask(wimp_poll_flags mask)
{
  LOCALS.mask = mask;
}

void event_set_earliest(os_t earliest)
{
  LOCALS.earliest = earliest;
}

void event_zero_pollword(void)
{
  *((int *) LOCALS.pollword) = 0;
}

void *event_get_pollword(void)
{
  return LOCALS.pollword;
}

/* ----------------------------------------------------------------------- */

wimp_event_no event_poll(wimp_block *block)
{
  wimp_poll_flags poll_flags;
  wimp_event_no   event_no;

  poll_flags = LOCALS.mask | event_wimp_get_mask();
  if ((poll_flags & wimp_MASK_NULL) == 0 && LOCALS.earliest > 0)
  {
    event_no = wimp_poll_idle(poll_flags,
                              block,
                              LOCALS.earliest,
                              LOCALS.pollword);
  }
  else
  {
    event_no = wimp_poll(poll_flags, block, LOCALS.pollword);
  }

  (void) event_dispatch_wimp(event_no, block);

  return event_no;
}
