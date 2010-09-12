/* --------------------------------------------------------------------------
 *    Name: save.c
 * Purpose: Save dialogue
 * Version: $Id: save.c,v 1.9 2009-05-20 21:20:41 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/osbyte.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/oserror.h"
#include "appengine/base/errors.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"
#include "appengine/types.h"
#include "appengine/wimp/window.h"

#include "appengine/dialogues/save.h"

/* ----------------------------------------------------------------------- */

/* Icons for window "save" */
enum
{
  SAVE_I_ICON     = 0,
  SAVE_W_FILENAME = 1,
  SAVE_B_CANCEL   = 2,
  SAVE_B_SAVE     = 3,
};

/* ----------------------------------------------------------------------- */

typedef struct save_t
{
  dialogue_t          dialogue; /* base class */
  char               *file_name;
  bits                file_type;
  save__save_handler *save_handler;
}
save_t;

/* ----------------------------------------------------------------------- */

static wimp_mouse_state saving_close_menu;

/* ----------------------------------------------------------------------- */

static event_wimp_handler save__event_mouse_click,
                          save__event_user_drag_box;

/* ----------------------------------------------------------------------- */

dialogue_t *save__create(void)
{
  save_t *s;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return NULL;

  dialogue__construct(&s->dialogue, "save", SAVE_B_SAVE, SAVE_B_CANCEL);

  dialogue__set_handlers(&s->dialogue,
                         save__event_mouse_click,
                         NULL,
                         NULL);

  return &s->dialogue;
}

void save__destroy(dialogue_t *d)
{
  save_t *s;

  s = (save_t *) d;

  dialogue__destruct(&s->dialogue);

  free(s->file_name);
  free(s);
}

/* ----------------------------------------------------------------------- */

static int save__event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  save_t       *s;

  NOT_USED(event_no);

  pointer = &block->pointer;
  s       = handle;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case SAVE_B_SAVE:
      {
        wimp_w win;
        char   file_name[256]; /* Careful Now */
        char  *p;

        win = dialogue__get_window(&s->dialogue);

        icon_get_text(win, SAVE_W_FILENAME, file_name);

        /* if it contains a dot or a colon, let it through */
        for (p = file_name; *p != '\0' && *p != '.' && *p != ':'; p++)
          ;
        if (*p == '\0')
        {
          oserror__report(0, "warn.drag.to.save");
          dialogue__keep_open(&s->dialogue);
        }
        else
        {
          s->save_handler(&s->dialogue, file_name);
        }
      }
      break;
    }
  }
  else if (pointer->buttons & (wimp_DRAG_SELECT | wimp_DRAG_ADJUST))
  {
    switch (pointer->i)
    {
    case SAVE_I_ICON:
      {
        char sprite[osspriteop_NAME_LIMIT];

        sprintf(sprite, "file_%3x", s->file_type);

        drag_icon(pointer->w, pointer->i,
                  pointer->pos.x, pointer->pos.y,
                  sprite);

        saving_close_menu = pointer->buttons;

        event_register_wimp_handler(wimp_USER_DRAG_BOX,
                                    event_ANY_WINDOW, event_ANY_ICON,
                                    save__event_user_drag_box, handle);

        break;
      }
    }
  }

  return event_HANDLED;
}

static int save__event_user_drag_box(wimp_event_no event_no, wimp_block *block, void *handle)
{
  save_t      *s;
  wimp_pointer pointer;
  wimp_w       w;

  NOT_USED(event_no);
  NOT_USED(block);

  s = handle;

  event_deregister_wimp_handler(wimp_USER_DRAG_BOX,
                                event_ANY_WINDOW, event_ANY_ICON,
                                save__event_user_drag_box, handle);

  drag_icon_stop();

  wimp_get_pointer_info(&pointer);

  w = dialogue__get_window(&s->dialogue);

  if (pointer.w != w)
  {
    char         file_name[256]; /* Careful Now */
    wimp_message message;

    icon_get_text(w, SAVE_W_FILENAME, file_name);

    message.data.data_xfer.w         = pointer.w;
    message.data.data_xfer.i         = pointer.i;
    message.data.data_xfer.pos.x     = pointer.pos.x;
    message.data.data_xfer.pos.y     = pointer.pos.y;
    message.data.data_xfer.est_size  = -1;
    message.data.data_xfer.file_type = s->file_type;
    strcpy(message.data.data_xfer.file_name, str_leaf(file_name));

    message.your_ref = 0;
    message.action = message_DATA_SAVE;

    message.size = wimp_SIZEOF_MESSAGE_HEADER((
                     offsetof(wimp_message_data_xfer, file_name) +
                     strlen(message.data.data_xfer.file_name) + 1 + 3) & ~3);

    wimp_send_message_to_window(wimp_USER_MESSAGE,
                               &message,
                                pointer.w,
                                pointer.i);
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

error save__set_file_name(dialogue_t *d, const char *file_name)
{
  save_t *s = (save_t *) d;
  wimp_w  win;

  /* record the filename so we can restore it when Cancel is pressed */

  free(s->file_name);
  s->file_name = NULL;

  s->file_name = str_dup(file_name);
  if (s->file_name == NULL)
    return error_OOM;

  win = dialogue__get_window(d);

  icon_set_text(win, SAVE_W_FILENAME, s->file_name);

  return error_OK;
}

void save__set_file_type(dialogue_t *d, bits file_type)
{
  save_t *s = (save_t *) d;
  wimp_w  win;

  s->file_type = file_type;

  win = dialogue__get_window(d);

  icon_validation_printf(win, SAVE_I_ICON,
                         "Ni_icon;Sfile_%3x", s->file_type);
}

void save__set_save_handler(dialogue_t *d, save__save_handler *save_handler)
{
  save_t *s = (save_t *) d;

  s->save_handler = save_handler;
}

/* ----------------------------------------------------------------------- */

int save__should_close_menu(void)
{
  return saving_close_menu & wimp_DRAG_SELECT;
}
