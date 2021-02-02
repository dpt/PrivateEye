/* --------------------------------------------------------------------------
 *    Name: clipboard.c
 * Purpose: Clipboard
 * ----------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/strings.h"
#include "appengine/wimp/event.h"

#include "globals.h"
#include "viewer.h"

#include "clipboard.h"

/* ----------------------------------------------------------------------- */

static event_message_handler message_claim_entity,
                             message_data_request;

/* ----------------------------------------------------------------------- */

static void register_event_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_CLAIM_ENTITY, message_claim_entity },
    { message_DATA_REQUEST, message_data_request },
  };

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               NULL);
}

result_t clipboard_init(void)
{
  register_event_handlers(1);

  return result_OK;
}

void clipboard_fin(void)
{
  register_event_handlers(0);
}

/* ----------------------------------------------------------------------- */

static int message_claim_entity(wimp_message *message, void *handle)
{
  NOT_USED(handle);

  if (message->sender != GLOBALS.task_handle)
    if (message->data.claim_entity.flags == wimp_CLAIM_CLIPBOARD)
      clipboard_release(); /* we no longer own it */

  return event_HANDLED;
}

static int message_data_request(wimp_message *message, void *handle)
{
  bits file_type;
  bits *types_list;
  wimp_message datasave;
  int i;

  NOT_USED(handle);

  if (GLOBALS.clipboard_viewer != NULL &&
      message->data.data_request.flags == wimp_DATA_REQUEST_CLIPBOARD)
  {
    /* The clipboard was requested and we own the clipboard. */

    types_list = message->data.data_request.file_types;

    file_type = GLOBALS.clipboard_viewer->drawable->image->display.file_type;

    /* If the file type is in the list, or if the list is empty, then send. */

    for (i = 0; types_list[i] != file_type && types_list[i] != 0xffffffffu; i++)
      ;

    if (types_list[0] == 0xffffffffu || types_list[i] != 0xffffffffu)
    {
      /* They requested a format we can provide. Respond with a DataSave. */

      datasave.data.data_xfer.w         = message->data.data_xfer.w;
      datasave.data.data_xfer.i         = message->data.data_xfer.i;
      datasave.data.data_xfer.pos.x     = message->data.data_xfer.pos.x;
      datasave.data.data_xfer.pos.y     = message->data.data_xfer.pos.y;
      datasave.data.data_xfer.est_size  = GLOBALS.clipboard_viewer->drawable->image->display.file_size;
      datasave.data.data_xfer.file_type = file_type;
      strcpy(datasave.data.data_xfer.file_name,
             str_leaf(GLOBALS.clipboard_viewer->drawable->image->file_name));

      datasave.size = wimp_SIZEOF_MESSAGE_HEADER((
                        offsetof(wimp_message_data_xfer, file_name) +
                        strlen(datasave.data.data_xfer.file_name) + 1 + 3) & ~3);
      datasave.your_ref = message->my_ref;
      datasave.action = message_DATA_SAVE;

      wimp_send_message(wimp_USER_MESSAGE, &datasave, message->sender);
    }
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void clipboard_claim(wimp_w w)
{
  wimp_message message;

  /* We always broadcast this, even if we already own the clipboard (see
   * RISCOS Ltd. Clipboard TechNote). */

  /* Claim the clipboard, even if we already own it */
  message.size     = sizeof(wimp_full_message_claim_entity);
  message.your_ref = 0;
  message.action   = message_CLAIM_ENTITY;
  message.data.claim_entity.flags = wimp_CLAIM_CLIPBOARD;
  wimp_send_message(wimp_USER_MESSAGE, &message, wimp_BROADCAST);

  GLOBALS.clipboard_viewer = viewer_find(w);
}

void clipboard_release(void)
{
  GLOBALS.clipboard_viewer = NULL;
}

osbool clipboard_own(void)
{
  return GLOBALS.clipboard_viewer != NULL;
}

