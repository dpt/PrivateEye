/* --------------------------------------------------------------------------
 *    Name: init.c
 * Purpose: Tag cloud
 * Version: $Id: init.c,v 1.1 2010-01-05 22:39:36 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/wimp/help.h"
#include "appengine/dialogues/info.h"
#include "appengine/base/messages.h"
#include "appengine/wimp/window.h"
#include "appengine/gadgets/tag-cloud.h"
#include "appengine/dialogues/name.h"

#include "impl.h"

static struct
{
  wimp_w      main_w;
  wimp_w      toolbar_w;
  dialogue_t *newtag_d;
  dialogue_t *renametag_d;
  dialogue_t *taginfo_d;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static int tag_cloud__refcount = 0;

error tag_cloud__init(void)
{
  if (tag_cloud__refcount++ == 0)
  {
    LOCALS.main_w      = window_create("tag_cloud");
    LOCALS.toolbar_w   = window_create("tag_cloud_t");
    LOCALS.newtag_d    = name__create("tag_new");
    LOCALS.renametag_d = name__create("tag_rename");
    LOCALS.taginfo_d   = info__create("tag_info");
    info__set_padding(LOCALS.taginfo_d, 128); /* 64 on either side */
  }

  return error_OK;
}

void tag_cloud__fin(void)
{
  if (--tag_cloud__refcount == 0)
  {
    info__destroy(LOCALS.taginfo_d);
    name__destroy(LOCALS.renametag_d);
    name__destroy(LOCALS.newtag_d);
    wimp_delete_window(LOCALS.toolbar_w);
    wimp_delete_window(LOCALS.main_w);
  }
}

/* ----------------------------------------------------------------------- */

wimp_w tag_cloud__get_main_window(void)
{
  return LOCALS.main_w;
}

wimp_w tag_cloud__get_toolbar_window(void)
{
  return LOCALS.toolbar_w;
}

dialogue_t *tag_cloud__get_newtag_dialogue(void)
{
  return LOCALS.newtag_d;
}

dialogue_t *tag_cloud__get_renametag_dialogue(void)
{
  return LOCALS.renametag_d;
}

dialogue_t *tag_cloud__get_taginfo_dialogue(void)
{
  return LOCALS.taginfo_d;
}
