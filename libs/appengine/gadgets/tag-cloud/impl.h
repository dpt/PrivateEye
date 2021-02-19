/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#ifndef TAG_CLOUD_IMPL_H
#define TAG_CLOUD_IMPL_H

#include "oslib/font.h"
#include "oslib/wimp.h"

#include "datastruct/bitvec.h"
#include "datastruct/atom.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

#include "appengine/gadgets/tag-cloud.h"

#define MAXTOKEN 64

typedef struct tag_cloud_entry
{
  atom_t           atom;
  int              count;
}
tag_cloud_entry;

typedef struct tag_cloud_scale_data
{
  int              min;
  int              scale;
}
tag_cloud_scale_data;

typedef struct tag_cloud_boxes
{
  os_box          *boxes; /* array of boxes, one per tag */
  int              allocated;
}
tag_cloud_boxes;

typedef struct tag_cloud_dim
{
  int              length[2]; /* length for regular, bold */
  int              scale;
}
tag_cloud_dim;

typedef struct tag_cloud_dims
{
  int              spacespacerlength; /* length of a single space in font units */
  int              bulletspacerlength; /* length of the gap string in font units */

  tag_cloud_dim  *dims;
  int             allocated;
}
tag_cloud_dims;

/* Cached lengths (widths) of highlighted tags. */
typedef struct tag_cloud_lengths
{
  int              widest;    /* font units */
  int             *length;
  int              allocated;
}
tag_cloud_lengths;

typedef struct tag_cloud_paintstring
{
  char            *string;
  int              allocated;
  int              used;
}
tag_cloud_paintstring;

typedef struct tag_cloud_layout_data
{
  int              width;     /* OS units */
  int              height;    /* OS units */

  int              font_size; /* OS units */
  int              leading;   /* OS units */
  int              padding;   /* OS units */

  int              descender; /* font units */

  font_f           font;      /* regular font */
  font_f           bold_font; /* bold font */

  tag_cloud_paintstring paintstring;
  tag_cloud_dims        dims;
  tag_cloud_lengths     lengths;
  tag_cloud_boxes       boxes;
}
tag_cloud_layout_data;

typedef bitvec_t *tag_cloud_highlight_data;

typedef struct tag_cloud_hover_data
{
  atom_t           index; /* tag to underline */
  int              last_index;
  const char      *pointer_shape_name; /* or NULL if none */
}
tag_cloud_hover_data;

enum
{
  tag_cloud_FLAG_NEW_DATA         = (1 << 0),
  tag_cloud_FLAG_NEW_HIGHLIGHTS   = (1 << 1),
  tag_cloud_FLAG_NEW_HOVER        = (1 << 2),
  tag_cloud_FLAG_NEW_SORT         = (1 << 3), /* new sort applied */
  tag_cloud_FLAG_NEW_DISPLAY      = (1 << 4),
  tag_cloud_FLAG_NEW_SHADE        = (1 << 5),

  tag_cloud_FLAG_NEW_ALL          = (1 << 6) - 1,

  tag_cloud_FLAG_SHADED           = (1 << 6),
  tag_cloud_FLAG_SORT_SEL_FIRST   = (1 << 7),
  tag_cloud_FLAG_TOOLBAR          = (1 << 8),
  tag_cloud_FLAG_TOOLBAR_NOT_EVER = (1 << 9),
  tag_cloud_FLAG_LAYOUT_PREPED    = (1 << 10),
};

typedef unsigned int tag_cloud_flags;

#define MAXSCALES (5)

struct tag_cloud
{
  tag_cloud_config          config;

  tag_cloud_flags           flags;

  wimp_w                    main_w; /* cloned per instance */
  wimp_w                    toolbar_w;
  wimp_menu                *main_m; // should not need to be per-instance

  tag_cloud_newtagfn       *newtag;
  tag_cloud_deletetagfn    *deletetag;
  tag_cloud_renametagfn    *renametag;
  tag_cloud_tagfn          *tag;
  tag_cloud_tagfn          *detag;
  tag_cloud_tagfilefn      *tagfile;
  tag_cloud_tagfilefn      *detagfile;
  tag_cloud_eventfn        *event;
  void                     *opaque;

  tag_cloud_key_handler_fn *key_handler;
  void                     *key_handler_arg; // -> key_handler_opaque

  atom_set_t               *dict;

  tag_cloud_entry          *entries; /* of length ntags */
  atom_t                   *sorted;  /* of length ntags */
  int                       ntags;

  int                       sort_type;
  int                       order_type;
  int                       scaletab[MAXSCALES];
  int                       display_type;
  int                       scaling_type;

  tag_cloud_scale_data      scale;

  tag_cloud_layout_data     layout;

  tag_cloud_highlight_data  highlight;

  tag_cloud_hover_data      hover;

  int                       menued_tag_index; /* indexes 'entries' */

  struct
  {
    int                     last_sort_type;
    int                     last_order_type;
  }
  sort;
};

void tag_cloud_internal_set_handlers(int reg, tag_cloud *tc);

/* layout.c */
result_t tag_cloud_layout_prepare(tag_cloud *tc);
result_t tag_cloud_layout(tag_cloud *tc, int width);
void tag_cloud_layout_reset(tag_cloud *tc);
/* Discard cached metrics. */
void tag_cloud_layout_discard(tag_cloud *tc);
int tag_cloud_hit(tag_cloud *tc, int x, int y);

/* init.c */
wimp_w tag_cloud_get_main_window(void);
wimp_w tag_cloud_get_toolbar_window(void);
dialogue_t *tag_cloud_get_newtag_dialogue(void);
dialogue_t *tag_cloud_get_renametag_dialogue(void);
dialogue_t *tag_cloud_get_taginfo_dialogue(void);

/* hover.c */
void tag_cloud_hover_init(tag_cloud *tc);
void tag_cloud_hover(tag_cloud *tc, int x, int y);
void tag_cloud_restore_pointer_shape(tag_cloud *tc);
void tag_cloud_hover_toggle(tag_cloud *tc);

/* redraw.c */
enum
{
  tag_cloud_SYNC_OPEN_TOP         = (1 << 0),
  tag_cloud_SYNC_EXTENT           = (1 << 1),
  tag_cloud_SYNC_REDRAW           = (1 << 2),
  tag_cloud_SYNC_REDRAW_IF_LAYOUT = (1 << 3),
};

typedef unsigned int tag_cloud_sync_flags;

void tag_cloud_sync(tag_cloud *tc, tag_cloud_sync_flags flags);
void tag_cloud_redraw(tag_cloud *tc);
void tag_cloud_kick_extent(tag_cloud *tc);

/* open.c */
void tag_cloud_post_reopen(tag_cloud *tc);

/* highlight.c */
int tag_cloud_is_highlighted(tag_cloud *tc, int index);

/* toolbar.c */
void tag_cloud_attach_toolbar(tag_cloud *tc);
void tag_cloud_detach_toolbar(tag_cloud *tc);
void tag_cloud_toggle_toolbar(tag_cloud *tc);
void tag_cloud_toolbar_adjust_extent(const tag_cloud *tc, os_box *box);

#endif /* TAG_CLOUD_IMPL_H */
