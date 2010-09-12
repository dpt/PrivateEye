/* --------------------------------------------------------------------------
 *    Name: event-message.c
 * Purpose: Event library
 * Version: $Id: event-message.c,v 1.2 2010-01-06 00:36:20 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/help.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/wimp/event.h"

#include "event-message.h"

/* ----------------------------------------------------------------------- */

typedef struct
{
  bits                   msg_no;
  wimp_w                 w;
  wimp_i                 i;
  event_message_handler *handler;
  void                  *handle;
}
message_handler_element;

typedef struct
{
  message_handler_element *entries;   /* the array */
  int                      nentries;  /* number of entries active */
  int                      allocated; /* number of entries allocated */
}
message_handler_array;

static message_handler_array client_handler_array = { NULL, 0, 0 };

/* ----------------------------------------------------------------------- */

static int event_message(wimp_event_no event_no,
                         wimp_block   *block,
                         void         *handle)
{
  static const struct
  {
    bits   action;
    size_t offset;
  }
  locs[] =
  {
    /* note that a many of these offsets are the same... */

    { message_DATA_SAVE,     offsetof(wimp_message_data_xfer, w)           },
    { message_DATA_SAVE_ACK, offsetof(wimp_message_data_xfer, w)           },
    { message_DATA_LOAD,     offsetof(wimp_message_data_xfer, w)           },
    { message_DATA_LOAD_ACK, offsetof(wimp_message_data_xfer, w)           },
    { message_DATA_REQUEST,  offsetof(wimp_message_data_request, w)        },
    { message_DRAGGING,      offsetof(wimp_message_dragging, w)            },
    { message_HELP_REQUEST,  offsetof(help_message_request, w)             },

    /* For the next two events we treat the submenu pointer as a window
     * handle. This allows us to attach events to windows being shown as
     * submenu entries and windows being closed after being opened as menus.
     */
    { message_MENU_WARNING,  offsetof(wimp_message_menu_warning, sub_menu) },
    { message_MENUS_DELETED, offsetof(wimp_message_menus_deleted, menu)    },
  };

  int                      i;
  wimp_w                   w;
  message_handler_array   *v;
  message_handler_element *e;

  NOT_USED(event_no);
  NOT_USED(handle);

  assert(event_no == wimp_USER_MESSAGE ||
         event_no == wimp_USER_MESSAGE_RECORDED ||
         event_no == wimp_USER_MESSAGE_ACKNOWLEDGE);
  assert(handle == NULL);

  i = bsearch_uint(&locs[0].action,
                    NELEMS(locs),
                    sizeof(locs[0]),
                    block->message.action);
  if (i >= 0)
    w = *((wimp_w *) (((char *) &block->message.data) + locs[i].offset));
  else
    w = event_ANY_WINDOW;

  v = &client_handler_array;

  for (e = v->entries; e < v->entries + v->nentries; e++)
  {
    if (e->msg_no == block->message.action)
    {
      if (e->w == event_ANY_WINDOW || e->w == w)
      {
        if (e->handler(&block->message, e->handle))
          return event_HANDLED;
      }
    }
  }

  /* client handlers haven't handled it so ... */

  if (event_no == wimp_USER_MESSAGE ||
      event_no == wimp_USER_MESSAGE_RECORDED)
  {
    switch (block->message.action)
    {
    case message_QUIT:
      exit(0); /* no return */
      break;
    }
  }

  return event_NOT_HANDLED;
}

/* ----------------------------------------------------------------------- */

/* internal interfaces */

static int set_handlers(int reg)
{
  static const bits msg_nos[] =
  {
    wimp_USER_MESSAGE,
    wimp_USER_MESSAGE_RECORDED,
    wimp_USER_MESSAGE_ACKNOWLEDGE
  };

  event_register_wimp_handler_prototype *fn;
  int i;

  fn = (reg) ? event_register_wimp_handler : event_deregister_wimp_handler;

  for (i = 0; i < NELEMS(msg_nos); i++)
    fn(msg_nos[i], event_ANY_WINDOW, event_ANY_ICON, event_message, NULL);

  return 0; /* success */
}

int event_initialise_message(void)
{
  set_handlers(1);

  return 0; /* success */
}

void event_finalise_message(void)
{
  set_handlers(0);

  free(client_handler_array.entries);
}

/* ----------------------------------------------------------------------- */

/* external interfaces */

static int compare_message_handler_elements(const void *va, const void *vb)
{
  const message_handler_element *a = va;
  const message_handler_element *b = vb;

  int score_a;
  int score_b;

  /* sort elements ascending by msg_no */

  if (a->msg_no < b->msg_no)
    return -1;
  else if (a->msg_no > b->msg_no)
    return +1;

  /* within the same msg_no, sort elements ascending by specificity (most
   * specific handlers are sorted earlier) */

  score_a = ((a->w != event_ANY_WINDOW) << 1) |
            ((a->i != event_ANY_ICON)   << 0);
  score_b = ((b->w != event_ANY_WINDOW) << 1) |
            ((b->i != event_ANY_ICON)   << 0);

  /* the higher priority elements need to sort to an earlier position in the
   * array, so the return value here is inverted */

  if (score_a < score_b)
    return +1;
  else if (score_a > score_b)
    return -1;
  else return 0;
}

int event_register_message_handler(bits                   msg_no,
                                   wimp_w                 w,
                                   wimp_i                 i,
                                   event_message_handler *handler,
                                   const void            *handle)
{
  message_handler_array   *v;
  message_handler_element *e;
  message_handler_element *end;
  wimp_MESSAGE_LIST(2)     list;

  v = &client_handler_array;

  /* check for an already registered handler */

  for (e = v->entries, end = v->entries + v->nentries; e < end; e++)
  {
    if (e->msg_no  == msg_no  &&
        e->w       == w       &&
        e->i       == i       &&
        e->handler == handler &&
        e->handle  == handle)
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

  e->msg_no  = msg_no;
  e->w       = w;
  e->i       = i;
  e->handler = handler;
  e->handle  = (void *) handle; /* cast away const */

  v->nentries++;

  list.messages[0] = msg_no;
  list.messages[1] = 0;
  wimp_add_messages((wimp_message_list *) &list);

  /* sort the entries so that more specific handlers come before less
   * specific ones */

  qsort(v->entries, v->nentries, sizeof(*v->entries),
        compare_message_handler_elements);

  return 0; /* success */
}

static void delete_message_handler_element(message_handler_array   *v,
                                           message_handler_element *e)
{
  size_t n;

  n = (v->entries + v->nentries) - (e + 1);

  if (n)
    memmove(e, e + 1, sizeof(*v->entries) * n);

  v->nentries--;
}

int event_deregister_message_handler(bits                   msg_no,
                                     wimp_w                 w,
                                     wimp_i                 i,
                                     event_message_handler *handler,
                                     const void            *handle)
{
  message_handler_array   *v;
  message_handler_element *e;
  message_handler_element *end;

  v = &client_handler_array;

  /* find the handler */

  for (e = v->entries, end = v->entries + v->nentries; e < end; e++)
  {
    if (e->msg_no  == msg_no  &&
        e->w       == w       &&
        e->i       == i       &&
        e->handler == handler &&
        e->handle  == handle)
    {
      break;
    }
  }

  /* if it exists, delete it */

  if (e == end)
    return 0; /* handler not found */

  /* got one. delete it */

  delete_message_handler_element(v, e);

  return 0; /* success */
}
