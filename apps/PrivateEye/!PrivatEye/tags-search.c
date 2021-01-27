/* --------------------------------------------------------------------------
 *    Name: tags-search.c
 * Purpose: Searching for tagged images
 * ----------------------------------------------------------------------- */

#ifdef EYE_TAGS

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "databases/filename-db.h"
#include "databases/tag-db.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/base/os.h"
#include "appengine/datastruct/array.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/help.h"

#include "globals.h"
#include "iconnames.h"
#include "tags-common.h"

#include "tags-search.h"

/* ----------------------------------------------------------------------- */

typedef struct
{
  int *indices; /* tag cloud indices */
  int  nindices;
  int  allocated;
}
Indices;

static struct
{
  wimp_w       tags_search_w;
  tags_common  common;
  tag_cloud   *tc;
  Indices      indices;
}
LOCALS;

/* ----------------------------------------------------------------------- */

static error tag(tag_cloud *tc, int index, void *opaque)
{
  error    err;
  Indices *ind = &LOCALS.indices;

  NOT_USED(opaque);

  if (array_grow((void **) &ind->indices,
                 sizeof(*ind->indices),
                 ind->nindices, /* used */
                 &ind->allocated,
                 1,
                 8))
  {
    err = error_OOM;
    goto failure;
  }

  ind->indices[ind->nindices++] = index;

//  tag_cloud_add_highlight(tc, index);

  err = tag_cloud_highlight(tc, ind->indices, ind->nindices);
  if (err)
    return err;

  return error_OK;


failure:

  return err;
}

static error detag(tag_cloud *tc, int index, void *opaque)
{
  error    err;
  Indices *ind = &LOCALS.indices;
  int      i;

  NOT_USED(opaque);

  for (i = 0; i < ind->nindices; i++)
    if (ind->indices[i] == index)
      break;

  if (i == ind->nindices)
    return error_OK; /* index not found */

  array_delete_element(ind->indices,
                       sizeof(*ind->indices),
                       ind->nindices,
                       i);

  ind->nindices--;

//  tag_cloud_remove_highlight(tc, index);

  err = tag_cloud_highlight(tc, ind->indices, ind->nindices);
  if (err)
    return err;

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static event_wimp_handler tags_search_event_mouse_click;

/* ----------------------------------------------------------------------- */

static void tags_search_set_handlers(int reg)
{
  static const event_wimp_handler_spec wimp_handlers[] =
  {
    { wimp_MOUSE_CLICK, tags_search_event_mouse_click },
  };

  event_register_wimp_group(reg,
                            wimp_handlers,
                            NELEMS(wimp_handlers),
                            LOCALS.tags_search_w,
                            event_ANY_ICON,
                            NULL);
}

/* ----------------------------------------------------------------------- */

static void tags_search_lazyfin(int force);

/* ----------------------------------------------------------------------- */

static int tags_search_refcount = 0;

error tags_search_init(void)
{
  if (tags_search_refcount++ == 0)
  {
    error err;

    /* dependencies */

    err = help_init();
    if (err)
      return err;

    err = tags_common_init();
    if (err)
      return err;

    LOCALS.tags_search_w = window_create("tags_search");

    tags_search_set_handlers(1);

    err = help_add_window(LOCALS.tags_search_w, "tags_search");
    if (err)
      return err;
  }

  return error_OK;
}

void tags_search_fin(void)
{
  if (--tags_search_refcount == 0)
  {
    free(LOCALS.indices.indices);

    tags_search_lazyfin(1); /* force shutdown */

    help_remove_window(LOCALS.tags_search_w);

    tags_search_set_handlers(0);

    tags_common_fin();

    help_fin();
  }
}

/* ----------------------------------------------------------------------- */

/* The 'lazy' init/fin functions provide lazy initialisation. */

static int tags_search_lazyrefcount = 0;

static error tags_search_lazyinit(void)
{
  error err;

  if (tags_search_lazyrefcount++ == 0)
  {
    tag_cloud_config conf;
    tag_cloud       *tc = NULL;

    err = tags_common_lazyinit();
    if (err)
      goto Failure;

    conf.size    = GLOBALS.choices.tagcloud.size;
    conf.leading = GLOBALS.choices.tagcloud.leading;
    conf.padding = GLOBALS.choices.tagcloud.padding;
    conf.scale   = GLOBALS.choices.tagcloud.scale;

    tc = tag_cloud_create(tag_cloud_CREATE_FLAG_TOOLBAR_DISABLED, &conf);
    if (tc == NULL)
    {
      err = error_OOM;
      goto Failure;
    }

    tag_cloud_set_handlers(tc,
                           tags_common_add_tag,
                           tags_common_delete_tag,
                           tags_common_rename_tag,
                           tag,
                           detag,
                           tags_common_tagfile,
                           tags_common_detagfile,
                           tags_common_event,
                           &LOCALS.common);

    /* tag_cloud_set_key_handler(tc, keyhandler, &LOCALS.common); */

    LOCALS.common.db          = tags_common_get_db(); /* FIXME: Seems grotty. */
    LOCALS.common.indextotag  = NULL;
    LOCALS.common.nindextotag = 0;

    LOCALS.tc                 = tc;

    err = tags_common_set_tags(tc, &LOCALS.common);
    if (err)
      goto Failure;
  }

  return error_OK;

Failure:

  return err;
}

static void tags_search_lazyfin(int force)
{
  if (tags_search_lazyrefcount == 0)
    return;

  if (force)
    tags_search_lazyrefcount = 1;

  if (--tags_search_lazyrefcount == 0)
  {
    tag_cloud_destroy(LOCALS.tc);

    tags_common_lazyfin(0); /* don't pass 'force' in */
  }
}

/* ----------------------------------------------------------------------- */

/* This is very rough at the moment. */
static error tags_search_search(void)
{
  error         err;
  int           cont;
  char          buf[256];
  filenamedb_t *fdb;

  fdb = tags_common_get_filename_db();

  /* no mode switch yet (any/all) */

  cont = 0;
  do
  {
    /* this function matches all tags */
    err = tagdb_enumerate_ids_by_tags(tags_common_get_db(),
                      (tagdb_tag_t *) LOCALS.indices.indices,
                                      LOCALS.indices.nindices,
                                     &cont,
                                      buf, sizeof(buf));
    if (err)
      return err;

    if (cont)
    {
      os_error   *oserr;
      const char *filename;

      filename = filenamedb_get(fdb, buf);
      if (filename)
      {
        fileswitch_object_type type;

        oserr = xosfile_read_no_path(filename, &type, NULL, NULL, NULL, NULL);
        if (oserr)
          printf("- <%s> <%s> error <%s>\n", buf, filename, oserr->errmess);
        else if (type == fileswitch_NOT_FOUND)
          printf("- <%s> not found on disc\n", filename);
        else
          printf("- <%s> ok\n", filename);
      }
      else
      {
        printf("- <%s> not in db\n", buf);
        /* not in filenamedb */
      }
    }
  }
  while (cont);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

static int tags_search_event_mouse_click(wimp_event_no event_no,
                                         wimp_block   *block,
                                         void         *handle)
{
  wimp_pointer *pointer;

  NOT_USED(event_no);
  NOT_USED(handle);

  pointer = &block->pointer;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
      case TAGS_SEARCH_B_SEARCH:
        tags_search_search();
        break;

      case TAGS_SEARCH_B_CANCEL:
        break;
    }

    if (pointer->i == TAGS_SEARCH_B_SEARCH ||
        pointer->i == TAGS_SEARCH_B_CANCEL)
    {
#if 0
      if (pointer->buttons & wimp_CLICK_SELECT)
        tags_search_close_dialogue();
      else
        tags_search_update_dialogue();
#endif
    }
  }

  return event_HANDLED;
}

static error search_attach_child(wimp_w parent, wimp_w child, wimp_i icon)
{
  wimp_window_state         pstate;
  wimp_icon_state           istate;
  wimp_window_nesting_flags linkage;
  wimp_window_state         cstate;
  wimp_outline              coutline;

  pstate.w = parent;
  wimp_get_window_state(&pstate);

  istate.w = parent;
  istate.i = icon;
  wimp_get_icon_state(&istate);

  cstate.w = child;
  wimp_get_window_state(&cstate);

  coutline.w = child;
  wimp_get_window_outline(&coutline);

  /* record window furniture sizes */
  coutline.outline.x0 -= cstate.visible.x0;
  coutline.outline.y0 -= cstate.visible.y0;
  coutline.outline.x1 -= cstate.visible.x1;
  coutline.outline.y1 -= cstate.visible.y1;

  /* scr to wrk: wrk_x = scr_x + xscroll - visible_x0 */
  /* wrk to scr: scr_x = wrk_x - xscroll + visible_x0 */

  cstate.visible.x0 = istate.icon.extent.x0 - pstate.xscroll + pstate.visible.x0;
  cstate.visible.y0 = istate.icon.extent.y0 - pstate.yscroll + pstate.visible.y1;
  cstate.visible.x1 = istate.icon.extent.x1 - pstate.xscroll + pstate.visible.x0;
  cstate.visible.y1 = istate.icon.extent.y1 - pstate.yscroll + pstate.visible.y1;

  /* adjust for furniture */
  cstate.visible.x0 -= coutline.outline.x0;
  cstate.visible.y0 -= coutline.outline.y0;
  cstate.visible.x1 -= coutline.outline.x1;
  cstate.visible.y1 -= -coutline.outline.y0; /* the title bar is removed when
                                                we open the window, so use
                                                the bottom edge as a guide */

  cstate.next   = wimp_TOP;
  cstate.flags &= ~(wimp_WINDOW_AUTO_REDRAW |
                    wimp_WINDOW_BACK_ICON   |
                    wimp_WINDOW_CLOSE_ICON  |
                    wimp_WINDOW_TITLE_ICON  |
                    wimp_WINDOW_TOGGLE_ICON |
                    wimp_WINDOW_SIZE_ICON   |
                    wimp_WINDOW_HSCROLL);

  linkage = (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_LS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_BS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_RS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_TS_EDGE_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_XORIGIN_SHIFT) |
            (wimp_CHILD_LINKS_PARENT_WORK_AREA << wimp_CHILD_YORIGIN_SHIFT);

  wimp_open_window_nested_with_flags(&cstate, parent, linkage);

  return error_OK;
}

error tags_search_open(void)
{
  error             err;
  wimp_w            w;
  wimp_window_state state;

  /* load the databases, create tag cloud, etc. */

  err = tags_search_lazyinit();
  if (err)
    return err;

  w = LOCALS.tags_search_w;

  state.w = w;
  wimp_get_window_state(&state);
  if (state.flags & wimp_WINDOW_OPEN)
  {
    /* bring to front and gain focus */

    state.next = wimp_TOP;
    wimp_open_window((wimp_open *) &state);
  }
  else
  {
    /* opening for the first(?) time */

    /* tags_search_update_dialogue(); */

    window_open_at(w, AT_BOTTOMPOINTER);

    search_attach_child(w, tag_cloud_get_window_handle(LOCALS.tc), 2);
  }

  return error_OK;
}

#else

extern int dummy;

#endif /* EYE_TAGS */
