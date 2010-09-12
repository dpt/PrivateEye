/* --------------------------------------------------------------------------
 *    Name: event-wimp.c
 * Purpose: Event library
 * Version: $Id: event-wimp.c,v 1.1 2009-05-20 20:58:21 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"

#include "event-wimp.h"

/* ----------------------------------------------------------------------- */

#define MAX_WIMP_HANDLERS 0x14

/* ----------------------------------------------------------------------- */

typedef struct wimp_handler_element
{
  wimp_event_no       event_no;
  wimp_w              w;
  wimp_i              i;
  event_wimp_handler *handler;
  void               *handle;
}
wimp_handler_element;

typedef struct
{
  wimp_handler_element *entries;   /* the array */
  int                   nentries;  /* number of entries active */
  int                   allocated; /* number of entries allocated */
}
wimp_handler_array;

static wimp_handler_array client_handler_arrays[MAX_WIMP_HANDLERS] =
{
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
  { NULL, 0, 0 },
};

/* ----------------------------------------------------------------------- */

static wimp_poll_flags event_wimp_poll_mask = 0;

static void calculate_mask(void)
{
  static const struct
  {
    wimp_event_no   event_no;
    wimp_poll_flags yes;
    wimp_poll_flags no;
  }
  tests[] =
  {
    { wimp_NULL_REASON_CODE,  0,                   wimp_MASK_NULL     },
    { wimp_LOSE_CARET,        0,                   wimp_MASK_LOSE     },
    { wimp_GAIN_CARET,        0,                   wimp_MASK_GAIN     },
    { wimp_POLLWORD_NON_ZERO, wimp_GIVEN_POLLWORD, wimp_MASK_POLLWORD },
  };

  wimp_poll_flags flags;
  int             i;

  flags = 0;

  for (i = 0; i < NELEMS(tests); i++)
  {
    if (client_handler_arrays[tests[i].event_no].nentries > 0)
      flags |= tests[i].yes;
    else
      flags |= tests[i].no;
  }

   event_wimp_poll_mask = flags;
}

wimp_poll_flags event_wimp_get_mask(void)
{
  return event_wimp_poll_mask;
}

/* ----------------------------------------------------------------------- */

/* Event handlers */

/*
 * When we walk the list of event handlers we recalculate the upper bound on
 * every iteration to allow handlers to deregister themselves.
 *
 * There's an assumption that the event handler won't deregister its handler
 * then return NOT_HANDLED. In that case we'll end up skipping the next
 * element as the deletion shifts it into the current element's place.
 *
 * If the handler adds another handler for the same event, and so ends up in
 * the same queue, then presently it's appended to the end of the list so the
 * current event handler will pick it up too.
 */

/* event handler which deals with events with no associated handles */
static int wimp_handler_vague(wimp_event_no event_no,
                              wimp_block   *block)
{
  wimp_handler_array         *v;
  const wimp_handler_element *e;

  v = &client_handler_arrays[event_no];

  for (e = v->entries; e < v->entries + v->nentries; e++)
  {
    if (e->handler(event_no, block, e->handle))
      return event_HANDLED;
  }

  return event_NOT_HANDLED;
}

/* event handler which deals with events with window handles */
static int wimp_handler_window(wimp_event_no event_no,
                               wimp_block   *block)
{
  wimp_handler_array         *v;
  const wimp_handler_element *e;
  wimp_w                      w;

  v = &client_handler_arrays[event_no];

  w = block->redraw.w; /* all window handles are at the same offset */

  for (e = v->entries; e < v->entries + v->nentries; e++)
  {
    if (e->w == event_ANY_WINDOW || e->w == w)
    {
      if (e->handler(event_no, block, e->handle))
        return event_HANDLED;
    }
  }

  switch (event_no)
  {
  case wimp_OPEN_WINDOW_REQUEST:
    wimp_open_window(&block->open);
    return event_HANDLED;

  case wimp_CLOSE_WINDOW_REQUEST:
    wimp_close_window(block->close.w);
    return event_HANDLED;
  }

  return event_NOT_HANDLED;
}

/* event handler which deals with events with window and icon handles */
static int wimp_handler_window_and_icon(wimp_event_no event_no,
                                        wimp_block   *block)
{
  wimp_handler_array         *v;
  const wimp_handler_element *e;
  wimp_w                      w;
  wimp_i                      i;

  v = &client_handler_arrays[event_no];

  /* find which window and icon handles the event is for, if any */

  switch (event_no)
  {
  case wimp_MOUSE_CLICK:
    w = block->pointer.w;
    i = block->pointer.i;
    break;

  case wimp_KEY_PRESSED:
  case wimp_LOSE_CARET:
  case wimp_GAIN_CARET:
    w = block->key.w; /* all at the same offset */
    i = block->key.i;
    break;

  case wimp_SCROLL_REQUEST:
    w = block->scroll.w;
    i = block->scroll.i;
    break;

  default:
    return event_NOT_HANDLED;
  }

  for (e = v->entries; e < v->entries + v->nentries; e++)
  {
    if ((e->w == event_ANY_WINDOW || e->w == w) &&
        (e->i == event_ANY_ICON   || e->i == i))
    {
      if (e->handler(event_no, block, e->handle))
        return event_HANDLED;
    }
  }

  return event_NOT_HANDLED;
}

/* ----------------------------------------------------------------------- */

typedef int (wimp_handler)(wimp_event_no event_no,
                           wimp_block   *block);

static wimp_handler * const wimp_handlers[MAX_WIMP_HANDLERS] =
{
  /* wimp_NULL_REASON_CODE         */ wimp_handler_vague,
  /* wimp_REDRAW_WINDOW_REQUEST    */ wimp_handler_window,
  /* wimp_OPEN_WINDOW_REQUEST      */ wimp_handler_window,
  /* wimp_CLOSE_WINDOW_REQUEST     */ wimp_handler_window,
  /* wimp_POINTER_LEAVING_WINDOW   */ wimp_handler_window,
  /* wimp_POINTER_ENTERING_WINDOW  */ wimp_handler_window,
  /* wimp_MOUSE_CLICK              */ wimp_handler_window_and_icon,
  /* wimp_USER_DRAG_BOX            */ wimp_handler_vague,
  /* wimp_KEY_PRESSED              */ wimp_handler_window_and_icon,
  /* wimp_MENU_SELECTION           */ wimp_handler_vague,
  /* wimp_SCROLL_REQUEST           */ wimp_handler_window_and_icon, /* we differ from DeskLib here */
  /* wimp_LOSE_CARET               */ wimp_handler_window_and_icon,
  /* wimp_GAIN_CARET               */ wimp_handler_window_and_icon,
  /* wimp_POLLWORD_NON_ZERO        */ wimp_handler_vague,
  /* unused                        */ NULL,
  /* unused                        */ NULL,
  /* unused                        */ NULL,
  /* wimp_USER_MESSAGE             */ wimp_handler_vague,
  /* wimp_USER_MESSAGE_RECORDED    */ wimp_handler_vague,
  /* wimp_USER_MESSAGE_ACKNOWLEDGE */ wimp_handler_vague,
};

/* ----------------------------------------------------------------------- */

/* internal interfaces */

int event_dispatch_wimp(wimp_event_no event_no, wimp_block *block)
{
  return wimp_handlers[event_no](event_no, block);
}

void event_finalise_wimp(void)
{
  int i;

  /* iterate over all events */

  for (i = 0; i < MAX_WIMP_HANDLERS; i++)
    free(client_handler_arrays[i].entries);
}

/* ----------------------------------------------------------------------- */

/* external interfaces */

int event_register_wimp_handler(wimp_event_no       event_no,
                                wimp_w              w,
                                wimp_i              i,
                                event_wimp_handler *handler,
                                const void         *handle)
{
  wimp_handler_array   *v;
  wimp_handler_element *e;
  wimp_handler_element *end;

  v = &client_handler_arrays[event_no];

  /* check for an already registered handler */

  for (e = v->entries, end = v->entries + v->nentries; e < end; e++)
  {
    if (e->event_no == event_no &&
        e->w        == w        &&
        e->i        == i        &&
        e->handler  == handler  &&
        e->handle   == handle)
    {
      return 1; /* failure: already registered */
    }
  }

  /* create a new entry */

  e = end;
  if (e == v->entries + v->allocated)
  {
    int   n;
    void *newentries;

    /* doubling strategy */

    n = v->allocated * 2;
    if (n < 8)
      n = 8;

    newentries = realloc(v->entries, sizeof(*v->entries) * n);
    if (newentries == NULL)
      return 1; /* failure: out of memory */

    v->entries = newentries;
    v->allocated = n;

    e = v->entries + v->nentries;
  }

  e->event_no = event_no;
  e->w        = w;
  e->i        = i;
  e->handler  = handler;
  e->handle   = (void *) handle; /* cast away const */

  v->nentries++;

  calculate_mask();

  return 0; /* success */
}

static void delete_wimp_handler_element(wimp_handler_array   *v,
                                        wimp_handler_element *e)
{
  size_t n;

  n = (v->entries + v->nentries) - (e + 1);

  if (n)
    memmove(e, e + 1, sizeof(*v->entries) * n);

  v->nentries--;
}

int event_deregister_wimp_handler(wimp_event_no       event_no,
                                  wimp_w              w,
                                  wimp_i              i,
                                  event_wimp_handler *handler,
                                  const void         *handle)
{
  wimp_handler_array   *v;
  wimp_handler_element *e;
  wimp_handler_element *end;

  v = &client_handler_arrays[event_no];

  /* find the handler */

  for (e = v->entries, end = v->entries + v->nentries; e < end; e++)
  {
    if (e->event_no == event_no &&
        e->w        == w        &&
        e->i        == i        &&
        e->handler  == handler  &&
        e->handle   == handle)
    {
      break;
    }
  }

  /* if it exists, delete it */

  if (e == end)
    return 0; /* handler not found */

  /* got one. delete it */

  delete_wimp_handler_element(v, e);

  calculate_mask();

  return 0; /* success */
}

int event_deregister_wimp_handlers_for_window(wimp_w w)
{
  int i;

  /* iterate over all events which deal with windows */

  for (i = 0; i < MAX_WIMP_HANDLERS; i++)
  {
    wimp_handler_array   *v;
    wimp_handler_element *e;

    if (wimp_handlers[i] != wimp_handler_vague)
      continue;

    v = &client_handler_arrays[i];

    /* If we delete an element then the subsequent elements, if any, will be
     * shifted down over it. The entry pointer will remain the same so don't      * automatically increment it. */

    for (e = v->entries; e < v->entries + v->nentries; )
      if (e->w == w)
        delete_wimp_handler_element(v, e);
      else
        e++;
  }

  calculate_mask();

  return 0; /* success */
}
