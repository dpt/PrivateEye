/* --------------------------------------------------------------------------
 *    Name: dataxfer.h
 * Purpose: Data transfer
 * ----------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/base/bsearch.h"
#include "appengine/base/strings.h"
#include "appengine/dialogues/save.h"
#include "appengine/wimp/event.h"

#include "ffg.h"
#include "globals.h"
#include "thumbview.h"
#include "viewer.h"
#include "save.h"

#include "dataxfer.h"

/* ----------------------------------------------------------------------- */

static int last_datasave_ref;

/* ----------------------------------------------------------------------- */

static event_message_handler message_data_save,
                             message_data_save_ack,
                             message_data_load,
                             message_data_load_ack,
                             message_data_open;

/* ----------------------------------------------------------------------- */

static void register_event_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_DATA_SAVE,     message_data_save     },
    { message_DATA_SAVE_ACK, message_data_save_ack },
    { message_DATA_LOAD,     message_data_load     },
    { message_DATA_LOAD_ACK, message_data_load_ack },
    { message_DATA_OPEN,     message_data_open     },
  };

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               NULL);
}

result_t dataxfer_init(void)
{
  register_event_handlers(1);

  return result_OK;
}

void dataxfer_fin(void)
{
  register_event_handlers(0);
}

/* ----------------------------------------------------------------------- */

/* Acknowledge a Message_DataLoad or Message_DataOpen.
 *
 * We send this even if we failed to load a file since we still attempted to
 * load it. */
static void send_ack(wimp_message *message)
{
  message->your_ref = message->my_ref;
  message->action   = message_DATA_LOAD_ACK;
  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);
}

/* ----------------------------------------------------------------------- */

/* Another task has proposed to save to us - tell them to save to
 * <Wimp$Scrap>. */
static int message_data_save(wimp_message *message, void *handle)
{
  os_error *e;

  NOT_USED(handle);

  /* Ensure that the <Wimp$Scrap> system variable exists before we ask anyone
   * to use it.
   *
   * I previously tried to use xos_read_var_val_size() here but it didn't
   * error when expected... */
  e = xos_read_var_val("Wimp$Scrap",
                       0,
                      -1,
                       0,
                       os_VARTYPE_STRING,
                       NULL,
                       NULL,
                       NULL);
  if (e != NULL && e->errnum == error_VAR_CANT_FIND)
  {
    oserror_report(0, "error.no.scrap");
    return event_HANDLED;
  }

  /* Are we interested in this file type? */
  if (!image_is_loadable(message->data.data_xfer.file_type))
    if (!ffg_is_loadable(message->data.data_xfer.file_type))
      return event_NOT_HANDLED;

  /* We are - respond with Message_DataSaveAck. */
  strcpy(message->data.data_xfer.file_name, "<Wimp$Scrap>");
 
  /* Let the sender know that the data is not "secure" (won't be saved to disc.) */
  message->data.data_xfer.est_size = -1; 

  message->size     = wimp_SIZEOF_MESSAGE_HEADER((
                        offsetof(wimp_message_data_xfer, file_name) +
                        strlen(message->data.data_xfer.file_name) + 1 + 3) & ~3);
  message->your_ref = message->my_ref; /* reply */
  message->action   = message_DATA_SAVE_ACK;
  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

  last_datasave_ref = message->my_ref;

  return event_HANDLED;
}

/* We proposed a save to another task and it's responded. */
static int message_data_save_ack(wimp_message *message, void *handle)
{
  viewer_t *viewer;
  osbool    unsafe;

  NOT_USED(handle);

  /* Find out what we were attempting to send. If a save dialogue is open
   * then ask that, otherwise check to see if the clipboard is claimed. */
  viewer = viewer_savedlg_get();
  if (viewer == NULL)
  {
    viewer = GLOBALS.clipboard_viewer;
    if (viewer == NULL)
      return event_NOT_HANDLED;
  }

  /* If it's an unsafe transfer (i.e. app-to-app) then tell viewer_save() to
   * NOT change the image's filename. We don't trust est_size here in case
   * bad apps don't set it correctly, so check the file_name too. */
  unsafe = (message->data.data_xfer.est_size == -1) ||
           (strcmp(message->data.data_xfer.file_name, "<Wimp$Scrap>") == 0);

  /* Write the file. */
  if (viewer_save(viewer, message->data.data_xfer.file_name, unsafe))
    return event_HANDLED; /* failure */

  /* Tell the receiving app that the file is ready. */
  message->your_ref = message->my_ref;
  message->action   = message_DATA_LOAD;
  wimp_send_message(wimp_USER_MESSAGE_RECORDED, message, message->sender);

  save_done(); /* closes the dialogue if required */

  return event_HANDLED;
}

/* We've been invited to load a file. */
static int message_data_load(wimp_message *message, void *handle)
{
  result_t  err;
  osbool    unsafe;
  osbool    its_ffg;
  osbool    try_ffg;
  viewer_t *viewer;
  bits      load_addr;
  bits      exec_addr;

  NOT_USED(handle);

  /* Ignore applications */
  if (message->data.data_xfer.file_type > osfile_TYPE_DIR)
    return event_NOT_HANDLED;

  /* Ignore directories, unless thumbview is enabled. */
  if (message->data.data_xfer.file_type == osfile_TYPE_DIR)
  {
#ifdef EYE_THUMBVIEW
    result_t   err;
    thumbview *tv;

    err = thumbview_create(&tv);
    if (err)
    {
      /* FIXME: Report the error. */
      return event_HANDLED;
    }

    thumbview_load_dir(tv, message->data.data_xfer.file_name);

    thumbview_open(tv);

    send_ack(message);

    return event_HANDLED;
#else
    return event_NOT_HANDLED;
#endif /* EYE_THUMBVIEW */
  }

  /* Is it an app-to-app transfer? */
  unsafe = (last_datasave_ref != 0) &&
           (message->your_ref == last_datasave_ref) &&
           (message->data.data_xfer.est_size == -1);

  if ((its_ffg = ffg_apposite(message)) != FALSE)
  {
    try_ffg = 0;
  }
  else
  {
    try_ffg = 0;

    if (!image_is_loadable(message->data.data_xfer.file_type))
    {
      /* try file content (in case it's not filetyped properly) */
      /* this updates message->data.data_xfer.file_type (icky) */
      if (image_recognise(message) != 0)
      {
        /* try transloaders */
        if (ffg_is_loadable(message->data.data_xfer.file_type))
          try_ffg = 1;
        else
          return event_NOT_HANDLED;
      }
    }
  }

  send_ack(message);

  if (its_ffg)
  {
    ffg_complete(message);
  }
  else if (try_ffg)
  {
    ffg_convert(message);
    return event_HANDLED;
  }

  /* Is it targeting an exsting viewer? */
  viewer = viewer_find(message->data.data_xfer.w);
  if (viewer == NULL)
  {
    /* It's a new viewer. */
    err = viewer_create(&viewer);
    if (err != result_OK)
      return event_HANDLED; /* tried but failed */
  }
  else
  {
    /* It's an existing viewer, possibly modified. */
    if (!viewer_query_unload(viewer))
      return event_HANDLED;

    /* Remove the previous occupant. */
    viewer_unload(viewer);
  }

  osfile_read_no_path(message->data.data_xfer.file_name,
                     &load_addr,
                     &exec_addr,
                      NULL,
                      NULL);
  if (viewer_load(viewer,
                  message->data.data_xfer.file_name,
                  load_addr,
                  exec_addr,
                  unsafe,
                  FALSE))
  {
    /* report */
    viewer_destroy(viewer);
    return event_HANDLED; /* tried but failed */
  }

  viewer_open(viewer);

  /* Clean up after an app-to-app transfer. */
  if (unsafe)
  {
    last_datasave_ref = 0;
    if (strcmp(message->data.data_xfer.file_name, "<Wimp$Scrap>") == 0)
      remove("<Wimp$Scrap>");
    else
      oserror_report(0, "error.dataxfer.scrap", message->data.data_xfer.file_name);
  }

  return event_HANDLED;
}

static int message_data_load_ack(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static const struct
{
  bits    file_type;
  osbool *load;
}
enabled_file_types[] =
{
  /* Sorted by file type so we can binary search. */
  { 0x695 /* gif_FILE_TYPE  */,     &GLOBALS.choices.gif.load      },
  { 0xaff /* osfile_TYPE_DRAW */,   &GLOBALS.choices.drawfile.load },
  { 0xb60 /* png_FILE_TYPE */,      &GLOBALS.choices.png.load      },
  { 0xc85 /* jpeg_FILE_TYPE */,     &GLOBALS.choices.jpeg.load     },
  { 0xd94 /* artworks_FILE_TYPE */, &GLOBALS.choices.artworks.load },
  { 0xff9 /* osfile_TYPE_SPRITE */, &GLOBALS.choices.sprite.load   }
};

/* Should we respond to Message_DataOpen for the specified file type? */
static osbool should_load(bits file_type)
{
  int i;

  i = bsearch_uint(&enabled_file_types[0].file_type,
                    NELEMS(enabled_file_types),
                    sizeof(enabled_file_types[0]),
                    file_type);

  return i >= 0 && *enabled_file_types[i].load;
}

/* ----------------------------------------------------------------------- */

static int message_data_open(wimp_message *message, void *handle)
{
  result_t  err;
  viewer_t *viewer;
  osbool    is_template;
  bits      load_addr;
  bits      exec_addr;

  NOT_USED(handle);

  /* Ignore applications */
  if (message->data.data_xfer.file_type > osfile_TYPE_DIR)
    return event_NOT_HANDLED;

  if (!should_load(message->data.data_xfer.file_type))
    return event_NOT_HANDLED;
     
  /* When est_size is -2 this is a file being opened as a document template.
   * In this case we load the file then replace its full filename with the
   * leafname only. */
  is_template = (message->data.data_xfer.est_size == -2);

  send_ack(message);

  osfile_read_no_path(message->data.data_xfer.file_name,
                     &load_addr,
                     &exec_addr,
                      NULL,
                      NULL);
  viewer = viewer_find_by_attrs(message->data.data_xfer.file_name,
                                load_addr,
                                exec_addr);
  if (viewer)
  {
    /* This file is already loaded: bring its window to top. */
    window_open(viewer->main_w);
    return event_HANDLED;
  }

  err = viewer_create(&viewer);
  if (err != result_OK)
    return event_HANDLED; /* tried but failed */

  if (viewer_load(viewer,
                  message->data.data_xfer.file_name,
                  load_addr,
                  exec_addr,
                  FALSE,
                  is_template))
  {
    /* report */
    viewer_destroy(viewer);
    return event_HANDLED; /* tried but failed */
  }
 
  viewer_open(viewer);

  return event_HANDLED;
}
