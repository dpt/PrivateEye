/* --------------------------------------------------------------------------
 *    Name: save.c
 * Purpose: Save dialogue
 * ----------------------------------------------------------------------- */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/osbyte.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"
#include "oslib/wimpreadsysinfo.h"

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
  dialogue_t             dialogue; /* base class */
  char                  *file_name;
  bits                   file_type;
  size_t                 est_size;
  save_save_handler     *save_handler;
  save_dataxfer_handler *dataxfer_handler;
}
save_t;

/* ----------------------------------------------------------------------- */

static wimp_mouse_state saving_close_menu;

/* ----------------------------------------------------------------------- */

static event_wimp_handler save_event_mouse_click,
                          save_event_user_drag_box;

/* ----------------------------------------------------------------------- */

dialogue_t *save_create(void)
{
  save_t *s;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return NULL;

  dialogue_construct(&s->dialogue, "save", SAVE_B_SAVE, SAVE_B_CANCEL);

  dialogue_set_mouse_click_handler(&s->dialogue, save_event_mouse_click);

  return &s->dialogue;
}

void save_destroy(dialogue_t *d)
{
  save_t *s;

  s = (save_t *) d;

  dialogue_destruct(&s->dialogue);

  free(s->file_name);
  free(s);
}

/* ----------------------------------------------------------------------- */

static int save_event_mouse_click(wimp_event_no event_no,
                                  wimp_block   *block,
                                  void         *handle)
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

        win = dialogue_get_window(&s->dialogue);

        icon_get_text(win, SAVE_W_FILENAME, file_name);

        /* if it contains a dot or a colon, let it through */
        for (p = file_name; *p != '\0' && *p != '.' && *p != ':'; p++)
          ;
        if (*p == '\0')
        {
          oserror_report(0, "warn.drag.to.save");
          dialogue_keep_open(&s->dialogue);
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

        file_type_to_sprite_name(s->file_type, sprite);

        drag_icon(pointer->w, pointer->i,
                  pointer->pos.x, pointer->pos.y,
                  sprite);

        saving_close_menu = pointer->buttons;

        event_register_wimp_handler(wimp_USER_DRAG_BOX,
                                    event_ANY_WINDOW,
                                    event_ANY_ICON,
                                    save_event_user_drag_box,
                                    handle);

        break;
      }
    }
  }

  return event_HANDLED;
}

static int save_event_user_drag_box(wimp_event_no event_no,
                                    wimp_block   *block,
                                    void         *handle)
{
  save_t      *s;
  wimp_pointer pointer;
  wimp_w       w;
  wimp_t       dest_task;
  wimp_message message;
  char         file_name[256]; /* Careful Now */

  NOT_USED(event_no);
  NOT_USED(block);

  s = handle;

  event_deregister_wimp_handler(wimp_USER_DRAG_BOX,
                                event_ANY_WINDOW,
                                event_ANY_ICON,
                                save_event_user_drag_box,
                                handle);

  drag_icon_stop();

  wimp_get_pointer_info(&pointer);

  w = dialogue_get_window(&s->dialogue);

  /* do nothing if dropped in the save dialogue */
  if (pointer.w == w)
    return event_HANDLED;

  /* don't message ourselves unless dropped on the icon bar */
  if (pointer.w != wimp_ICON_BAR)
  {
    message.size     = 16;
    message.your_ref = 0;
    dest_task = wimp_send_message_to_window(wimp_USER_MESSAGE_ACKNOWLEDGE,
                                           &message,
                                            pointer.w,
                                            pointer.i);
    if (dest_task == wimpreadsysinfo_task(NULL))
      return event_HANDLED;
  }

  icon_get_text(w, SAVE_W_FILENAME, file_name);

  message.data.data_xfer.w         = pointer.w;
  message.data.data_xfer.i         = pointer.i;
  message.data.data_xfer.pos.x     = pointer.pos.x;
  message.data.data_xfer.pos.y     = pointer.pos.y;
  message.data.data_xfer.est_size  = s->est_size;
  message.data.data_xfer.file_type = s->file_type;
  strcpy(message.data.data_xfer.file_name, str_leaf(file_name));

  message.your_ref = 0; /* not a reply */
  message.action = message_DATA_SAVE;

  message.size = wimp_SIZEOF_MESSAGE_HEADER((
                   offsetof(wimp_message_data_xfer, file_name) +
                   strlen(message.data.data_xfer.file_name) + 1 + 3) & ~3);

  wimp_send_message_to_window(wimp_USER_MESSAGE_RECORDED,
                             &message,
                              pointer.w,
                              pointer.i);

  if (s->dataxfer_handler)
    s->dataxfer_handler(&s->dialogue, message.my_ref);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

result_t save_set_info(dialogue_t *d,
                 const char       *file_name,
                       bits        file_type,
                       size_t      bytes)
{
  save_t *s = (save_t *) d;
  wimp_w  w;
  char    sprname[osspriteop_NAME_LIMIT];

  /* record the filename so we can restore it when Cancel is pressed */

  free(s->file_name);
  s->file_name = NULL;

  s->file_name = str_dup(file_name);
  if (s->file_name == NULL)
    return result_OOM;

  s->file_type = file_type;
  s->est_size = bytes;

  w = dialogue_get_window(d);
  icon_set_text(w, SAVE_W_FILENAME, s->file_name);
  file_type_to_sprite_name(file_type, sprname);
  icon_validation_printf(w, SAVE_I_ICON, "Ni_icon;S%s", sprname);

  return result_OK;
}

void save_set_dataxfer_handler(dialogue_t *d, save_dataxfer_handler *dataxfer_handler)
{
  save_t *s = (save_t *) d;

  s->dataxfer_handler = dataxfer_handler;
}

void save_set_save_handler(dialogue_t *d, save_save_handler *save_handler)
{
  save_t *s = (save_t *) d;

  s->save_handler = save_handler;
}

/* ----------------------------------------------------------------------- */

void save_done(void)
{
  if (saving_close_menu & wimp_DRAG_SELECT)
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);
}
