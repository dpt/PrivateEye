/* --------------------------------------------------------------------------
 *    Name: eye.c
 * Purpose: Picture viewer
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
#include "appengine/base/bsearch.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/oserror.h"
#include "appengine/base/strings.h"
#include "appengine/vdu/screen.h"
#include "appengine/vdu/sprite.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"

#include "canvas.h"
#include "choicesdat.h"
#include "clipboard.h"
#include "dataxfer.h"
#include "display.h"
#include "effects.h"
#include "ffg.h"
#include "globals.h"
#include "iconbar.h"
#include "iconnames.h"          /* generated */
#include "imagecache.h"
#include "menunames.h"          /* not generated */
#include "privateeye.h"
#include "quit.h"
#include "rotate.h"
#include "scale.h"
#include "tags-search.h"
#include "tags.h"
#include "thumbview.h"
#include "viewer.h"
#include "zones.h"

/* ----------------------------------------------------------------------- */

static error initialise_substrate(void)
{
  typedef error (*initfn)(void);

  static const initfn initfns[] =
  {
#ifdef EYE_CANVAS
    canvas_substrate_init,
#endif
    display_substrate_init,
#ifdef EYE_TAGS
    tags_substrate_init,
#endif
#ifdef EYE_THUMBVIEW
    thumbview_substrate_init,
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

/* ----------------------------------------------------------------------- */

static event_message_handler message_quit,
                             message_save_desktop,
                             message_pre_quit,
                             message_palette_change,
                             message_mode_change;

/* ----------------------------------------------------------------------- */

static void register_event_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_QUIT,           message_quit           },
    { message_PRE_QUIT,       message_pre_quit       },
    { message_PALETTE_CHANGE, message_palette_change },
    { message_SAVE_DESKTOP,   message_save_desktop   },
    { message_MODE_CHANGE,    message_mode_change    },
  };

  event_register_message_group(reg,
                               message_handlers,
                               NELEMS(message_handlers),
                               event_ANY_WINDOW,
                               event_ANY_ICON,
                               NULL);
}

static error initialise_subsystems(void)
{
  typedef error (*initfn)(void);

  static const initfn initfns[] =
  {
    eye_icon_bar_init,
    rotate_init,
    effects_init,
#ifdef EYE_TAGS
    tags_search_init,
#endif
    display_init,
#ifdef EYE_CANVAS
    canvas_init,
#endif
#ifdef EYE_THUMBVIEW
    thumbview_init,
#endif
    clipboard_init,
    dataxfer_init
  };

  error err;
  int   i;

  /* initialise dependencies */

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
    dataxfer_fin,
    clipboard_fin,
#ifdef EYE_THUMBVIEW
    thumbview_fin,
#endif
#ifdef EYE_CANVAS
    canvas_fin,
#endif
    display_fin,
#ifdef EYE_TAGS
    tags_search_fin,
#endif
    effects_fin,
    rotate_fin,
    eye_icon_bar_fin,
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

  err = xosfile_read_no_path("<Choices$Write>." APPNAME,
                            &type,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
  if (err == NULL && type == fileswitch_IS_DIR)
    return; /* new format choices already exists */

  xosfile_create_dir("<Choices$Write>." APPNAME, 0);

  /* Convert Choices:Sliced.PrivateEye to Choices:PrivateEye.Choices. */

  err = xosfile_read_no_path("<Choices$Write>.Sliced." APPNAME,
                            &type,
                             NULL,
                             NULL,
                             NULL,
                             NULL);
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
                   0,
                   0,
                   0,
                   0,
                   NULL);
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

  err = initialise_substrate();
  if (err)
    goto Failure;

  open_messages(APPNAME "Res:Messages");

  install_choices();

  {
    int dynamic_size;
    int max_app;

    dynamic_size = -1;

    /* Read the maximum size of application space. If it's more than 28,640K
     * then use the application slot. This allows us to cope with Aemulor
     * restricting the maximum size of application space. */

    os_read_dynamic_area(os_DYNAMIC_AREA_APPLICATION_SPACE, 0, &max_app);
    if (max_app > 28640 * 1024)
    {
      const char *env;

      env = getenv("PrivateEye$Flex");
      if (env && strcmp(env, "WimpSlot") == 0)
        dynamic_size = 0; /* use application slot */
    }

    if (dynamic_size == -1)
    {
      dynamic_size = atoi(getenv("PrivateEye$DALimit"));
      if (dynamic_size > 0)
        dynamic_size *= 1024 * 1024; /* convert to megabytes */
      else
        dynamic_size = -1; /* system default maximum area size */
    }

    flex_init(APPNAME, 0, dynamic_size);
    flex_set_budge(1);
  }

  GLOBALS.task_handle = wimp_initialise(wimp_VERSION_RO38,
                                        message0("task"),
           (const wimp_message_list *) &messages,
                                       &GLOBALS.wimp_version);

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
    os_error              *e;
    fileswitch_object_type obj_type;
    bits                   load_addr;
    bits                   exec_addr;
    bits                   file_type;

    e = EC(xosfile_read_no_path(argv[argc],
                               &obj_type,
                               &load_addr,
                               &exec_addr,
                                NULL,
                                NULL));
    if (e)
    {
      oserror_report_block(e);
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
      viewer_t *viewer;

      err = viewer_create(&viewer); /* reports any failures itself */
      if (err == error_OK)
      {
        if (viewer_load(viewer, argv[argc], load_addr, exec_addr))
          viewer_destroy(viewer);
        else
          viewer_open(viewer);
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
    error_report(err);

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

static int message_save_desktop(wimp_message *message, void *handle)
{
  static const char run[] = "Run ";

  const char *dir;
  os_fw       file;

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

    wimp_send_message(wimp_KEY_PRESSED,
    (wimp_message *) &key,
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

static int kick_update_event_null_reason_code(wimp_event_no event_no,
                                               wimp_block   *block,
                                               void         *handle)
{
  NOT_USED(event_no);
  NOT_USED(block);
  NOT_USED(handle);

  viewer_update_all(viewer_UPDATE_EXTENT | viewer_UPDATE_REDRAW);

  event_deregister_wimp_handler(wimp_NULL_REASON_CODE,
                                event_ANY_WINDOW,
                                event_ANY_ICON,
                                kick_update_event_null_reason_code,
                                NULL);

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
                                event_ANY_WINDOW,
                                event_ANY_ICON,
                                kick_update_event_null_reason_code,
                                NULL);
  }

  return event_PASS_ON;
}
