/* --------------------------------------------------------------------------
 *    Name: help.c
 * Purpose: Interactive Help
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/help.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/bitwise.h"
#include "appengine/datastruct/atom.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/messages.h"

#include "appengine/wimp/help.h"

typedef struct help_element
{
  unsigned int obj;
  atom_t       atom;
}
help_element;

typedef struct
{
  help_element *entries;   /* the array */
  int           nentries;  /* number of entries active */
  int           allocated; /* number of entries allocated */

  atom_set_t   *atoms;
}
help_array;

/* ----------------------------------------------------------------------- */

static event_message_handler help_message_help_request;

/* ----------------------------------------------------------------------- */

static help_array windows;
static help_array menus;

/* ----------------------------------------------------------------------- */

static void register_event_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_HELP_REQUEST, help_message_help_request },
  };

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               NULL);
}

/* ----------------------------------------------------------------------- */

static unsigned int help_refcount = 0;

error help_init(void)
{
  error err;

  if (help_refcount == 0)
  {
    /* initialise */

    register_event_handlers(1);

    windows.atoms = atom_create();
    if (windows.atoms == NULL)
    {
      err = error_OOM;
      goto failure;
    }

    menus.atoms = atom_create();
    if (menus.atoms == NULL)
    {
      err = error_OOM;
      goto failure;
    }
  }

  help_refcount++;

  return error_OK;


failure:

  return err;
}

void help_fin(void)
{
  if (help_refcount == 0)
    return;

  if (--help_refcount == 0)
  {
    /* finalise */

    free(windows.entries);
    atom_destroy(windows.atoms);

    free(menus.entries);
    atom_destroy(menus.atoms);

    /* reset handles in case we're reinitialised */

    windows.entries = NULL;
    windows.atoms   = NULL;
    menus.entries   = NULL;
    menus.atoms     = NULL;

    register_event_handlers(0);
  }
}

/* ----------------------------------------------------------------------- */

// todo: cope with already-added window
static error add_element(help_array *arr, unsigned int obj, const char *name)
{
  error         err;
  help_element *e;

  if (arr->nentries + 1 > arr->allocated)
  {
    int   n;
    void *newentries;

    n = (int) power2gt(arr->allocated); /* doubling strategy */
    if (n < 8) // FIXME: Hoist growth constants.
      n = 8;

    newentries = realloc(arr->entries, n * sizeof(*arr->entries));
    if (newentries == NULL)
      return error_OOM; /* failure: out of memory */

    arr->entries   = newentries;
    arr->allocated = n;
  }

  e = arr->entries + arr->nentries;

  e->obj = obj;

  err = atom_new(arr->atoms, (const unsigned char *) name, strlen(name) + 1,
                 &e->atom);
  if (err != error_ATOM_NAME_EXISTS && err)
    return err;

  arr->nentries++;

  return error_OK;
}

static void remove_element(help_array *arr, unsigned int obj)
{
  int    i;
  size_t n;

  for (i = 0; i < arr->nentries; i++)
    if (arr->entries[i].obj == obj)
      break;

  if (i == arr->nentries)
    return; /* not found */

  n = arr->nentries - (i + 1);
  if (n)
    memmove(arr->entries + i,
            arr->entries + i + 1,
            n * sizeof(*arr->entries));

  arr->nentries--;
}

static const char *get(help_array *arr, unsigned int obj)
{
  int i;

  for (i = 0; i < arr->nentries; i++)
    if (arr->entries[i].obj == obj)
      return (const char *) atom_get(arr->atoms, arr->entries[i].atom, NULL);

  return NULL;
}

/* ----------------------------------------------------------------------- */

error help_add_window(wimp_w w, const char *name)
{
  return add_element(&windows, (unsigned int) w, name);
}

void help_remove_window(wimp_w w)
{
  remove_element(&windows, (unsigned int) w);
}

/* ----------------------------------------------------------------------- */

error help_add_menu(wimp_menu *m, const char *name)
{
  return add_element(&menus, (unsigned int) m, name);
}

void help_remove_menu(wimp_menu *m)
{
  remove_element(&menus, (unsigned int) m);
}

/* ----------------------------------------------------------------------- */

static int help_message_help_request(wimp_message *message, void *handle)
{
  help_message_request *request;
  const char           *partial_token;
  const char           *type;
  int                   i;
  wimp_selection        selection;
  wimp_menu            *lastmenu = NULL;
  char                  help_token[64]; /* Careful Now */
  const char           *name;
  help_message_reply   *reply;
  wimp_message          reply_message;
  int                   shaded = 0;
  const char           *messagestring;

  NOT_USED(handle);

  request = (help_message_request *) &message->data;

  partial_token = get(&windows, (unsigned int) request->w);
  type = "hw"; /* window */

  if (partial_token == NULL) /* not a window - check for menu */
  {
    wimp_get_menu_state(wimp_GIVEN_WINDOW_AND_ICON,
                       &selection,
                        request->w,
                        request->i);

    if (selection.items[0] == -1)
      return event_NOT_HANDLED; /* not found */

    lastmenu = menu_last();

    partial_token = get(&menus, (unsigned int) lastmenu);
    if (partial_token == NULL)
      return event_NOT_HANDLED; /* not found - give up */

    type = "hm"; /* menu */
  }

  sprintf(help_token, "%s.%s", type, partial_token);

  switch (type[1]) /* careful! */
  {
  case 'w':
    /* Window - append the icon name to the token, if available. */

    name = icon_get_name(request->w, request->i);
    if (*name != '\0')
    {
      strcat(help_token, ".");
      strcat(help_token, name);
    }
    else if (request->i > wimp_ICON_WINDOW)
    {
      sprintf(help_token + strlen(help_token), ".%d", request->i);
    }
    /* else it's the workarea */

    // test for shadedness
    break;

  case 'm':
  {
    char       buffer[4]; /* buffer for numbers */
    int        seldepth;
    wimp_menu *curmenu;

    /* Menu - append the selection list to the token. */

    for (i = 0; selection.items[i] != -1; i++)
    {
      sprintf(buffer, ".%d", selection.items[i]);
      strcat(help_token, buffer);
    }

    seldepth = i;

    /* Since we've got this far the leading (seldepth - 1) entries must all
     * have valid sub-menu pointers. Follow them to get a pointer to the
     * current menu. */

    curmenu = lastmenu;
    for (i = 0; i < seldepth - 1; i++)
      curmenu = curmenu->entries[selection.items[i]].sub_menu;

    /* Now we have the menu which the entry is located upon, look at the
     * entry's flags to see if we should append 'g' to indicate shaded. */

    shaded = (curmenu->entries[selection.items[i]].icon_flags & wimp_ICON_SHADED);
  }
  }

  if (shaded)
    strcat(help_token, "g");

  messagestring = message0(help_token);
  if (messagestring[0] == '\0')
  {
    char *r;

    /* no message - strip off the trailing element and try again */

    r = strrchr(help_token, '.');
    if (r != NULL)
      *r = '\0';

    messagestring = message0(help_token);
  }

  reply = (help_message_reply *) &reply_message.data;
  strncpy(reply->reply, messagestring, sizeof(reply->reply));

  reply_message.your_ref = message->my_ref;
  reply_message.action = message_HELP_REPLY;

  reply_message.size = wimp_SIZEOF_MESSAGE_HEADER((
                         offsetof(help_message_reply, reply) +
                         strlen(reply->reply) + 1 + 3) & ~3);

  /* Previously I would send this help reply message directly back to the
   * originator using the task handle in the source message. Then I found I
   * would get invalid task handle errors whilst dragging in PrivateEye's
   * image rotate window whilst interactive help was active. The task handle
   * did not look valid.
   *
   * The Toolbox always broadcasts help replies, so I'll do the same.
   */

  wimp_send_message(wimp_USER_MESSAGE, &reply_message, wimp_BROADCAST);

  return event_HANDLED;
}
