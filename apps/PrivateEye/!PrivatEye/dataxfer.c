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

#include "dataxfer.h"

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

static int message_data_save(wimp_message *message, void *handle)
{
  os_error *e;

  NOT_USED(handle);

  /* Ensure that Wimp$Scrap exists before we ask anyone to save data to it.
   */

  /* This version didn't error when expected... */
  /*e = xos_read_var_val_size("Wimp$Scrap",
                            0,
                            os_VARTYPE_STRING,
                            NULL,
                            NULL,
                            NULL);*/

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

  /* If we're interested, send DataSaveAck. */
  if (!image_is_loadable(message->data.data_xfer.file_type))
    if (!ffg_is_loadable(message->data.data_xfer.file_type))
      return event_NOT_HANDLED;

  strcpy(message->data.data_xfer.file_name, "<Wimp$Scrap>");
  message->data.data_xfer.est_size = -1; /* Let the sender know that the data
                                          * is unsafe (not saved to disc.) */

  message->size     = wimp_SIZEOF_MESSAGE_HEADER((
                        offsetof(wimp_message_data_xfer, file_name) +
                        strlen(message->data.data_xfer.file_name) + 1 + 3) & ~3);
  message->your_ref = message->my_ref;
  message->action   = message_DATA_SAVE_ACK;
  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

  return event_HANDLED;
}

static int message_data_save_ack(wimp_message *message, void *handle)
{
  viewer_t *viewer;

  NOT_USED(handle);

  /* FIXME: It may be incorrect to assume that the current_viewer will be
   * valid at this point. */
  viewer = GLOBALS.current_viewer;
  if (viewer == NULL)
    return event_NOT_HANDLED;

  if (viewer_save(viewer, message->data.data_xfer.file_name))
    return event_HANDLED; /* failure */

  message->your_ref = message->my_ref;
  message->action   = message_DATA_LOAD;
  wimp_send_message(wimp_USER_MESSAGE_RECORDED, message, message->sender);

  if (save_should_close_menu())
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int message_data_load(wimp_message *message, void *handle)
{
  result_t  err;
  osbool    its_ffg;
  osbool    try_ffg;
  viewer_t *viewer;
  bits      load_addr;
  bits      exec_addr;

  NOT_USED(handle);

  if (message->data.data_xfer.file_type > 0x1000)
    return event_NOT_HANDLED;

  if (message->data.data_xfer.file_type == 0x1000)
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

    /* acknowledge - even if we fail, we still tried to load it */
    message->your_ref = message->my_ref;
    message->action   = message_DATA_LOAD_ACK;
    wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

    return event_HANDLED;
#else
    return event_NOT_HANDLED;
#endif /* EYE_THUMBVIEW */
  }

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

  /* acknowledge - even if we fail, we still tried to load it */
  message->your_ref = message->my_ref;
  message->action   = message_DATA_LOAD_ACK;
  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

  if (its_ffg)
  {
    ffg_complete(message);
  }
  else if (try_ffg)
  {
    ffg_convert(message);
    return event_HANDLED;
  }

  viewer = viewer_find(message->data.data_xfer.w);
  if (viewer == NULL)
  {
    /* a new display */
    err = viewer_create(&viewer);
    if (err != result_OK)
      return event_HANDLED; /* tried but failed */
  }
  else
  {
    /* an existing display, possibly modified */
    if (!viewer_query_unload(viewer))
      return event_HANDLED;

    /* unload the previous occupant */
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
                  exec_addr))
  {
    /* report */
    viewer_destroy(viewer);
    return event_HANDLED; /* tried but failed */
  }

  viewer_open(viewer);

  if (strcmp(message->data.data_xfer.file_name, "<Wimp$Scrap>") == 0)
  {
    remove(message->data.data_xfer.file_name);
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
ShouldLoad_map[] =
{
  /* Sorted by file type so we can binary search. */
  { 0x695 /* gif_FILE_TYPE  */,     &GLOBALS.choices.gif.load      },
  { 0xaff /* osfile_TYPE_DRAW */,   &GLOBALS.choices.drawfile.load },
  { 0xb60 /* png_FILE_TYPE */,      &GLOBALS.choices.png.load      },
  { 0xc85 /* jpeg_FILE_TYPE */,     &GLOBALS.choices.jpeg.load     },
  { 0xd94 /* artworks_FILE_TYPE */, &GLOBALS.choices.artworks.load },
  { 0xff9 /* osfile_TYPE_SPRITE */, &GLOBALS.choices.sprite.load   }
};

/* Should we respond to DataOpen messages for the specified file type? */
static osbool should_load(bits file_type)
{
  int i;

  i = bsearch_uint(&ShouldLoad_map[0].file_type,
                    NELEMS(ShouldLoad_map),
                    sizeof(ShouldLoad_map[0]),
                    file_type);

  return i >= 0 && *ShouldLoad_map[i].load;
}

/* ----------------------------------------------------------------------- */

static int message_data_open(wimp_message *message, void *handle)
{
  viewer_t *viewer;
  bits      load_addr;
  bits      exec_addr;

  NOT_USED(handle);

  if (message->data.data_xfer.file_type > 0xfff)
    return event_NOT_HANDLED;

  if (!should_load(message->data.data_xfer.file_type))
    return event_NOT_HANDLED;

  /* acknowledge - even if we fail, we still tried to load it */
  message->your_ref = message->my_ref;
  message->action   = message_DATA_LOAD_ACK;
  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

  osfile_read_no_path(message->data.data_xfer.file_name,
                     &load_addr,
                     &exec_addr,
                      NULL,
                      NULL);

  viewer = viewer_find_by_attrs(message->data.data_xfer.file_name,
                                load_addr,
                                exec_addr);
  if (viewer == NULL)
  {
    result_t err;

    err = viewer_create(&viewer);
    if (err != result_OK)
      return event_HANDLED; /* tried but failed */

    if (viewer_load(viewer,
                    message->data.data_xfer.file_name,
                    load_addr,
                    exec_addr))
    {
      /* report */
      viewer_destroy(viewer);
      return event_HANDLED; /* tried but failed */
    }
    else
    {
      viewer_open(viewer);
    }
  }
  else
  {
    /* already loaded: bring window to top */
    window_open(viewer->main_w);
  }

  return event_HANDLED;
}
