/* --------------------------------------------------------------------------
 *    Name: tags.c
 * Purpose: Tag Cloud test
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
#include "oslib/os.h"
#include "oslib/osgbpb.h"

#include "appengine/types.h"
#include "appengine/base/messages.h"
#include "appengine/base/os.h"
#include "appengine/base/oserror.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/icon.h"
#include "appengine/wimp/menu.h"
#include "appengine/wimp/window.h"

#include "app.h"
#include "globals.h"
#include "iconbar.h"
#include "menunames.h"          /* not generated */
#include "makecloud.h"

/* ----------------------------------------------------------------------- */

static event_message_handler message_quit,
                             message_save_desktop;

/* ----------------------------------------------------------------------- */

static void register_event_handlers(int reg)
{
  static const event_message_handler_spec message_handlers[] =
  {
    { message_QUIT,           message_quit           },
    { message_SAVE_DESKTOP,   message_save_desktop   },
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
    icon_bar__init,
    tag_cloud__init,
    makecloud_init
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
    makecloud_fin,
    tag_cloud__fin,
    icon_bar__fin
  };

  int i;

  for (i = 0; i < NELEMS(finfns); i++)
    finfns[i]();
}

/* ----------------------------------------------------------------------- */

int main(void)
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

      env = getenv(APPNAME "$Flex");
      if (env && strcmp(env, "WimpSlot") == 0)
        dynamic_size = 0; /* use application slot */
    }

    if (dynamic_size == -1)
    {
      dynamic_size = atoi(getenv(APPNAME "$DALimit"));
      if (dynamic_size > 0)
        dynamic_size *= 1024 * 1024; /* convert to megabytes */
      else
        dynamic_size = -1; /* system default maximum area size */
    }

    flex_init(APPNAME, 0, dynamic_size);
    flex_set_budge(1);
  }

  GLOBALS.task_handle = wimp_initialise(wimp_VERSION_RO38, message0("task"), (const wimp_message_list *) &messages, &GLOBALS.wimp_version);

  /* Event handling */
  event_initialise();

  /* Sprites */
  window_load_sprites(APPNAME "Res:Sprites");

  /* Window creation and event registration */
  templates_open(APPNAME "Res:Templates");

  err = initialise_subsystems();
  if (err)
    goto Failure;

  templates_close();

  /* Register the main event handlers last. This has the effect of putting
   * them at the end of the event handler lists, so they get called after
   * any other event handlers. */

  register_event_handlers(1);

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

  register_event_handlers(0);

  finalise_subsystems();

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
