/* --------------------------------------------------------------------------
 *    Name: eye.c
 * Purpose: Picture viewer
 * Version: $Id: eye.c,v 1.79 2010-01-06 01:58:53 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "flex.h"

#ifdef HierProf_PROFILE
#include "HierProf:HierProf.h"
#endif

#ifdef MemCheck_MEMCHECK
#include "MemCheck:MemCheck.h"
#endif

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/colourpicker.h"
#include "oslib/colourtrans.h"
#include "oslib/filer.h"
#include "oslib/fileswitch.h"
#include "oslib/hourglass.h"
#include "oslib/jpeg.h"
#include "oslib/os.h"
#include "oslib/osbyte.h"
#include "oslib/osfile.h"
#include "oslib/osfind.h"
#include "oslib/osfscontrol.h"
#include "oslib/osgbpb.h"
#include "oslib/osmodule.h"
#include "oslib/osspriteop.h"
#include "oslib/wimp.h"
#include "oslib/wimpspriteop.h"

#include "appengine/types.h"
#include "appengine/app/choices.h"
#include "appengine/dialogues/dcs-quit.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/oserror.h"
#include "appengine/dialogues/save.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"
#include "appengine/base/strings.h"
#include "appengine/base/bsearch.h"

#include "choicesdat.h"
#include "clipboard.h"
#include "display.h"
#include "effects.h"
#include "ffg.h"
#include "globals.h"
#include "hist.h"
#include "iconbar.h"
#include "iconnames.h"          /* generated */
#include "imagecache.h"
#include "menunames.h"          /* not generated */
#include "privateeye.h"
#include "quit.h"
#include "rotate.h"
#include "scale.h"
#include "tags.h"
#include "tags-search.h"
#include "thumbview.h"
#include "viewer.h"
#include "zones.h"

/* ----------------------------------------------------------------------- */

static event_message_handler message_quit,
                             message_data_save,
                             message_data_save_ack,
                             message_data_load,
                             message_data_load_ack,
                             message_data_open,
                             message_save_desktop,
                             message_pre_quit,
                             message_palette_change,
                             message_mode_change,
                             message_claim_entity,
                             message_data_request;

/* ----------------------------------------------------------------------- */

static void register_event_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_QUIT,           message_quit           },
    { message_DATA_SAVE,      message_data_save      },
    { message_DATA_SAVE_ACK,  message_data_save_ack  },
    { message_DATA_LOAD,      message_data_load      },
    { message_DATA_LOAD_ACK,  message_data_load_ack  },
    { message_DATA_OPEN,      message_data_open      },
    { message_PRE_QUIT,       message_pre_quit       },
    { message_PALETTE_CHANGE, message_palette_change },
    { message_SAVE_DESKTOP,   message_save_desktop   },
    { message_CLAIM_ENTITY,   message_claim_entity   },
    { message_DATA_REQUEST,   message_data_request   },
    { message_MODE_CHANGE,    message_mode_change    },
  };

  event_register_message_group(reg,
                               message_handlers, NELEMS(message_handlers),
                               event_ANY_WINDOW, event_ANY_ICON,
                               NULL);
}

static error initialise_subsystems(void)
{
  typedef error (*initfn)(void);

  static const initfn initfns[] =
  {
    dcs_quit__init,  /* in AppEngine */
    hist__init,
    icon_bar__init,
    rotate__init,
    effects__init,
#ifdef EYE_TAGS
    tags_search__init,
#endif
    display__init,   /* careful: this one depends on the earlier init calls */
#ifdef EYE_THUMBVIEW
    thumbview__init,
#endif
  };

  error err;
  int   i;

  for (i = 0; i < NELEMS(initfns); i++)
  {
    err = initfns[i]();
    if (err)
      return err;
  }

  return error_OK;
}

static void finalise_subsystems(void)
{
  typedef void (*finfn)(void);

  static const finfn finfns[] =
  {
#ifdef EYE_THUMBVIEW
    thumbview__fin,
#endif
    display__fin,
#ifdef EYE_TAGS
    tags_search__fin,
#endif
    effects__fin,
    rotate__fin,
    icon_bar__fin,
    hist__fin,
    dcs_quit__fin,
  };

  int i;

  for (i = 0; i < NELEMS(finfns); i++)
    finfns[i]();
}

/* ----------------------------------------------------------------------- */

/* PrivateEye used to be distibuted under the "Sliced Software" label and
 * consequently stored its choices in the file
 * <Choices$Write>.Sliced.PrivateEye. To support a more extensive choices
 * structure since version ?.?? it creates the directory
 * <Choices$Write>.PrivateEye from an old-style choices files if it finds
 * one. */
static void upgrade_choices(void)
{
  os_error              *err;
  fileswitch_object_type type;
  FILE                  *rd;
  FILE                  *wr;

  /* We can't use "Choices:XXX" here because it's a path variable which can
   * and does have multiple entries on it. Instead we use '<Choices$Write>.'
   */

  err = xosfile_read_no_path("<Choices$Write>." APPNAME, &type,
                             NULL, NULL, NULL, NULL);
  if (err == NULL && type == fileswitch_IS_DIR)
    return; /* new format choices already exists */

  xosfile_create_dir("<Choices$Write>." APPNAME, 0);

  /* Convert Choices:Sliced.PrivateEye to Choices:PrivateEye.Choices. */

  err = xosfile_read_no_path("<Choices$Write>.Sliced." APPNAME, &type,
                             NULL, NULL, NULL, NULL);
  if (err != NULL || type == fileswitch_NOT_FOUND)
    return; /* old format doesn't exist either */

  rd = fopen("<Choices$Write>.Sliced." APPNAME, "r");
  if (rd == NULL)
    return;

  wr = fopen("<Choices$Write>." APPNAME ".Choices", "w");
  if (wr == NULL)
    return;

  /* feof() doesn't seem to work if nothing's been read from the file yet, so
   * we need to check the result of fgets inside the loop. */

  while (!feof(rd))
  {
    char buffer[256];

    if (fgets(buffer, sizeof(buffer), rd))
    {
      char *dot;

      /* Turn "general.XXX" into "viewer.XXX". */

      dot = strchr(buffer, '.');
      if (dot && (dot - buffer == 7))
        if (memcmp(buffer, "general", 7) == 0)
          sprintf(buffer, "viewer%s", dot);

      fputs(buffer, wr);
    }
  }

  fclose(wr);
  fclose(rd);

  /* We'll leave the old version of the choices in place so old versions of
   * the app will continue to work. */
}

/* Copy the keymap into the choices directory, but only if it's newer than
 * the existing one. */
static void install_keymap(void)
{
  osfscontrol_copy(APPNAME "Res:Keys",
                   "<Choices$Write>." APPNAME ".Keys",
                   osfscontrol_COPY_FORCE | osfscontrol_COPY_NEWER,
                   0, 0, 0, 0, NULL);
}

static void install_choices(void)
{
  upgrade_choices();
  install_keymap();
}

/* ----------------------------------------------------------------------- */

int main(int argc, char *argv[])
{
  static const wimp_MESSAGE_LIST(2) messages =
  {{
    message_MENUS_DELETED, /* We register all other messages as they are
                              required using Wimp_AddMessages, but without
                              this here, ColourPicker blows things up. */
    message_QUIT
  }};

  error      err = error_OK;
  wimp_block poll;

#ifdef MemCheck_MEMCHECK
  MemCheck_Init();
  MemCheck_InterceptSCLStringFunctions();
  MemCheck_RegisterArgs(argc, argv);
  MemCheck_SetQuitting(0, 0);
#endif

#ifdef HierProf_PROFILE
  HierProf_ProfileAllFunctions();
#endif

#ifdef FORTIFY
  Fortify_EnterScope();
#endif

  open_messages(APPNAME "Res:Messages");

  install_choices();

  {
    int max_app;
    int dynamic_size;

    /* Read the maximum size of application space. If it's more than 28,640K
     * then use the application slot. This allows us to cope with Aemulor
     * restricting the maximum size of application space. */

    dynamic_size = -1; /* use a dynamic area by default */

    os_read_dynamic_area(os_DYNAMIC_AREA_APPLICATION_SPACE, 0, &max_app);
    if (max_app > 28640 * 1024)
    {
      const char *env;

      env = getenv("PrivateEye$Flex");
      if (env && strcmp(env, "WimpSlot") == 0)
        dynamic_size = 0; /* use application slot */
    }

    flex_init(APPNAME, 0, dynamic_size);
    flex_set_budge(1);
  }

  GLOBALS.task_handle = wimp_initialise(wimp_VERSION_RO38, message0("task"), (const wimp_message_list *) &messages, &GLOBALS.wimp_version);

  /* Event handling */
  event_initialise();

  cache_mode_vars();

  /* Sprites */
  window_load_sprites(APPNAME "Res:Sprites");

  /* Window creation and event registration */
  templates_open(APPNAME "Res:Templates");

  /* I would otherwise call this in initialise_subsystems, but the choices
   * must be made available before that is called. */
  err = choices_init();
  if (err)
    goto Failure;

  /* Choices */
  choices_create_windows(&privateeye_choices);
  choices_load(&privateeye_choices);

  err = initialise_subsystems();
  if (err)
    goto Failure;

  ffg_initialise(image_is_loadable);

  templates_close();

  /* Register the main event handlers last. This has the effect of putting
   * them at the end of the event handler lists, so they get called after
   * any other event handlers. */

  register_event_handlers(1);

  /* Process command line arguments */
  while (--argc)
  {
    os_error               *e;
    fileswitch_object_type  obj_type;
    bits                    load_addr;
    bits                    exec_addr;
    bits                    file_type;

    e = EC(xosfile_read_no_path(argv[argc],
                               &obj_type,
                               &load_addr,
                               &exec_addr,
                                NULL,
                                NULL));
    if (e)
    {
      oserror__report_block(e);
      continue;
    }

    if ((load_addr >> 20) == 0xfff) /* has file type */
      file_type = (load_addr >> 8) & 0xfff;
    else
      continue;

    /* Ignore files not found, unsupported file types, or directories given.
     */
    if (obj_type == fileswitch_IS_FILE && image_is_loadable(file_type))
    {
      viewer *viewer;

      err = viewer_create(&viewer); /* reports any failures itself */
      if (err == error_OK)
      {
        if (viewer_load(viewer, argv[argc], load_addr, exec_addr))
        {
          viewer_destroy(viewer);
        }
        else
        {
          viewer_open(viewer);
        }
      }
    }
  }

  while ((GLOBALS.flags & Flag_Quit) == 0)
  {
#ifndef NDEBUG
    if (inkey(INKEY_ALT) && inkey(INKEY_CTRL) && inkey(INKEY_SHIFT))
    {
#ifdef FORTIFY
      Fortify_DumpAllMemory();
#endif
      flex_save_heap_info("<" APPNAME "$Dir>.flex");
    }
#endif

    (void) event_poll(&poll);
  }

  viewer_close_all();

  imagecache_empty();

  register_event_handlers(0);

  ffg_finalise();

  finalise_subsystems();

  choices_destroy_windows(&privateeye_choices);

  choices_fin();

  event_finalise();

  wimp_close_down(GLOBALS.task_handle);

  close_messages();

#ifdef FORTIFY
  Fortify_LeaveScope();
  Fortify_OutputStatistics();
  Fortify_DumpAllMemory();
#endif

  exit(EXIT_SUCCESS);


Failure:

  if (err)
    error__report(err);

  exit(EXIT_FAILURE);
}

/* ----------------------------------------------------------------------- */

static int message_quit(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  GLOBALS.flags |= Flag_Quit;

  return event_HANDLED;
}

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
    oserror__report(0, "error.no.scrap");
    return event_HANDLED;
  }

  /* If we're interested, send DataSaveAck. */
  if (!image_is_loadable(message->data.data_xfer.file_type))
    if (!ffg_is_loadable(message->data.data_xfer.file_type))
      return event_NOT_HANDLED;

  strcpy(message->data.data_xfer.file_name, "<Wimp$Scrap>");
  message->data.data_xfer.est_size = -1; /* Let the sender know that the data
                                          * is unsafe (not saved to disc.) */

  message->your_ref = message->my_ref;
  message->action = message_DATA_SAVE_ACK;

  message->size = wimp_SIZEOF_MESSAGE_HEADER((
                    offsetof(wimp_message_data_xfer, file_name) +
                    strlen(message->data.data_xfer.file_name) + 1 + 3) & ~3);

  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

  return event_HANDLED;
}

static int message_data_save_ack(wimp_message *message, void *handle)
{
  viewer *viewer;

  NOT_USED(handle);

  /* FIXME: It may be incorrect to assume that the current_display_w will be
   * valid at this point. */
  viewer = viewer_find(GLOBALS.current_display_w);
  if (viewer == NULL)
    return event_NOT_HANDLED;

  if (viewer_save(viewer, message->data.data_xfer.file_name))
    return event_HANDLED; /* failure */

  message->your_ref = message->my_ref;
  message->action = message_DATA_LOAD;

  wimp_send_message(wimp_USER_MESSAGE_RECORDED, message, message->sender);

  if (save__should_close_menu())
    wimp_create_menu(wimp_CLOSE_MENU, 0, 0);

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

static int message_data_load(wimp_message *message, void *handle)
{
  osbool  ffg;
  viewer *viewer;
  bits    load_addr;
  bits    exec_addr;

  NOT_USED(handle);

  if (message->data.data_xfer.file_type > 0x1000)
    return event_NOT_HANDLED;

  if (message->data.data_xfer.file_type == 0x1000)
  {
#ifdef EYE_THUMBVIEW
    error      err;
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
    message->action   = message_DATA_LOAD_ACK;
    message->your_ref = message->my_ref;
    wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

    return event_HANDLED;
#else
    return event_NOT_HANDLED;
#endif /* EYE_THUMBVIEW */
  }

  ffg = 0;

  if (!image_is_loadable(message->data.data_xfer.file_type))
  {
    /* try file content (in case it's not filetyped properly) */
    /* this updates message->data.data_xfer.file_type (icky) */
    if (image_recognise(message) != 0)
    {
      /* try transloaders */
      if (ffg_is_loadable(message->data.data_xfer.file_type))
        ffg = 1;
      else
        return event_NOT_HANDLED;
    }
  }

  /* acknowledge - even if we fail, we still tried to load it */
  message->action   = message_DATA_LOAD_ACK;
  message->your_ref = message->my_ref;
  wimp_send_message(wimp_USER_MESSAGE, message, message->sender);

  if (ffg)
  {
    ffg_convert(message);
    return event_HANDLED;
  }

  viewer = viewer_find(message->data.data_xfer.w);
  if (viewer == NULL)
  {
    error err;

    /* a new display */
    err = viewer_create(&viewer);
    if (err != error_OK)
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

// why are these not just expressed directly in the call below?
static const size_t ShouldLoad_stride = sizeof(ShouldLoad_map[0]);
static const size_t ShouldLoad_nelems = sizeof(ShouldLoad_map) / sizeof(ShouldLoad_map[0]);

/* Should we respond to DataOpen messages for the specified file type? */
static osbool should_load(bits file_type)
{
  int i;

  i = bsearch_uint(&ShouldLoad_map[0].file_type,
                    ShouldLoad_nelems,
                    ShouldLoad_stride,
                    file_type);

  return i >= 0 && *ShouldLoad_map[i].load;
}

static int message_data_open(wimp_message *message, void *handle)
{
  viewer *viewer;
  bits    load_addr;
  bits    exec_addr;

  NOT_USED(handle);

  if (message->data.data_xfer.file_type > 0xfff)
    return event_NOT_HANDLED;

  if (!should_load(message->data.data_xfer.file_type))
    return event_NOT_HANDLED;

  /* acknowledge - even if we fail, we still tried to load it */
  message->action   = message_DATA_LOAD_ACK;
  message->your_ref = message->my_ref;
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
    error err;

    err = viewer_create(&viewer);
    if (err != error_OK)
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

static int message_save_desktop(wimp_message *message, void *handle)
{
  const char *dir;
  static const char run[] = "Run ";
  os_fw file;

  NOT_USED(handle);

  file = message->data.save_desktop.file;

  osgbpb_writew(file, (byte *) run, NELEMS(run) - 1);

  dir = getenv(APPNAME "$Dir");
  osgbpb_writew(file, (byte *) dir, strlen(dir));

  os_bputw('\n', file);

  /* FIXME: if (error) ack the message */

  return event_HANDLED;
}

static int message_pre_quit(wimp_message *message, void *handle)
{
  wimp_message_prequit *prequit;
  wimp_message          ack;

  NOT_USED(handle);

  prequit = (wimp_message_prequit *) &message->data;

  ack = *message;
  ack.your_ref = ack.my_ref;
  wimp_send_message(wimp_USER_MESSAGE_ACKNOWLEDGE, &ack, message->sender);

  if (!can_quit())
    return event_HANDLED;

  /* user says 'discard', so throw away all the images, restart shutdown and
   * quit */

  viewer_close_all();

  if ((prequit->flags & wimp_PRE_QUIT_TASK_ONLY) == 0)
  {
    wimp_key key;

    key.c = wimp_KEY_SHIFT | wimp_KEY_CONTROL | wimp_KEY_F12;

    wimp_send_message(wimp_KEY_PRESSED, (wimp_message *) &key,
                      message->sender);
  }

  GLOBALS.flags |= Flag_Quit;

  return event_HANDLED;
}

static int message_palette_change(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  viewer_update_all(viewer_UPDATE_COLOURS | viewer_UPDATE_REDRAW);

  return event_PASS_ON;
}

static int kick_update__event_null_reason_code(wimp_event_no event_no, wimp_block *block, void *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  viewer_update_all(viewer_UPDATE_EXTENT | viewer_UPDATE_REDRAW);

  event_deregister_wimp_handler(wimp_NULL_REASON_CODE,
                                event_ANY_WINDOW, event_ANY_ICON,
                                kick_update__event_null_reason_code, NULL);

  return event_HANDLED;
}

static int message_mode_change(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  cache_mode_vars();

  viewer_update_all(viewer_UPDATE_COLOURS | viewer_UPDATE_SCALING);

  if (GLOBALS.choices.viewer.size == 1)
  {
    /* Since we can't change the extent of windows on a mode change, we need
     * to register a callback to perform the work later on. If there was a
     * way to test if the screen resolution had actually changed then I could
     * avoid this. */

    event_register_wimp_handler(wimp_NULL_REASON_CODE,
                                event_ANY_WINDOW, event_ANY_ICON,
                                kick_update__event_null_reason_code, NULL);
  }

  return event_PASS_ON;
}

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
      strcpy(datasave.data.data_xfer.file_name, str_leaf(GLOBALS.clipboard_viewer->drawable->image->file_name));

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
