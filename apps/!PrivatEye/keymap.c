/* --------------------------------------------------------------------------
 *    Name: keymap.c
 * Purpose: Keymap
 * Version: $Id: keymap.c,v 1.11 2009-12-15 00:38:29 dpt Exp $
 * ----------------------------------------------------------------------- */

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/app/keymap.h"

#include "actions.h"
#include "privateeye.h"

#include "keymap.h"

/* ----------------------------------------------------------------------- */

/* KEEP THESE SORTED */

static const keymap_name_to_action common[] =
{
  { "Close",            Close                   },
  { "Help",             Help                    },
};

static const keymap_name_to_action viewer[] =
{
  { "ConvToSpr",        ConvToSpr               },
  { "Copy",             Copy                    },
  { "Effects",          Effects                 },
  { "Hist",             Hist                    },
  { "HorzFlip",         HorzFlip                },
  { "Info",             Info                    },
  { "Kill",             Kill                    },
#ifdef EYE_META
  { "MetaData",         MetaData                },
#endif
  { "NewView",          NewView                 },
  { "PanDown",          PanDown                 },
  { "PanLeft",          PanLeft                 },
  { "PanRandom",        PanRandom               },
  { "PanRight",         PanRight                },
  { "PanUp",            PanUp                   },
  { "Rotate",           Rotate                  },
  { "RotateLeft",       RotateLeft              },
  { "RotateRight",      RotateRight             },
  { "Save",             Save                    },
  { "Scale",            Scale                   },
  { "SourceInfo",       SourceInfo              },
  { "StepBackwards",    StepBackwards           },
  { "StepForwards",     StepForwards            },
//#ifdef EYE_TAGS
  { "Tags",             Tags                    },
//#endif
  { "VertFlip",         VertFlip                },
  { "ZoomIn",           ZoomIn                  },
  { "ZoomOut",          ZoomOut                 },
  { "ZoomReset",        ZoomReset               },
  { "ZoomToggle",       ZoomToggle              },
};

#ifdef EYE_TAGS
static const keymap_name_to_action tag_cloud[] =
{
  { "Cloud",            TagCloud_Cloud          },
  { "Commit",           TagCloud_Commit         },
  { "Info",             TagCloud_Info           },
  { "Kill",             TagCloud_Kill           },
  { "List",             TagCloud_List           },
  { "New",              TagCloud_New            },
  { "Rename",           TagCloud_Rename         },
  { "SmallCloud",       TagCloud_SmallCloud     },
  { "SortByCount",      TagCloud_SortByCount    },
  { "SortByName",       TagCloud_SortByName     },
  { "SortSelFirst",     TagCloud_SortSelFirst   },
  { "UnscaledCloud",    TagCloud_UnscaledCloud  },
};
#endif

#ifdef EYE_THUMBVIEW
static const keymap_name_to_action thumbview[] =
{
  { "ClearSelection",   Thumbview_ClearSelection },
  { "FullInfoHorz",     Thumbview_FullInfoHorz   },
  { "FullInfoVert",     Thumbview_FullInfoVert   },
  { "LargeThumbs",      Thumbview_LargeThumbs    },
  { "SelectAll",        Thumbview_SelectAll      },
  { "SmallThumbs",      Thumbview_SmallThumbs    },
  { "SortByCount",      Thumbview_SortByCount    },
  { "SortByName",       Thumbview_SortByName     },
};
#endif

static const keymap_section sections[] =
{
  { "Common",           common,         NELEMS(common)          },
#ifdef EYE_TAGS
  { "Tag Cloud",        tag_cloud,      NELEMS(tag_cloud)       },
#endif
#ifdef EYE_THUMBVIEW
  { "Thumbview",        thumbview,      NELEMS(thumbview)       },
#endif
  { "Viewer",           viewer,         NELEMS(viewer)          },
};

/* ----------------------------------------------------------------------- */

static keymap_t *keymap;

static int viewer_keymap__refcount = 0;

error viewer_keymap_init(void)
{
  error err;

  if (viewer_keymap__refcount++ == 0)
  {
    err = keymap__create("Choices:" APPNAME ".Keys",
                         sections, NELEMS(sections),
                        &keymap);

    return err;
  }

  return error_OK;
}

void viewer_keymap_fin(void)
{
  if (--viewer_keymap__refcount == 0)
  {
    keymap__destroy(keymap);
  }
}

int viewer_keymap_op(viewer_keymap_section section, wimp_key_no key_no)
{
  keymap_action a;

  a = keymap__action(keymap, section, key_no);
  if (a >= 0 || section == viewer_keymap_SECTION_COMMON)
    return a;

  return viewer_keymap_op(viewer_keymap_SECTION_COMMON, key_no);
}
