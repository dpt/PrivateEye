/* --------------------------------------------------------------------------
 *    Name: impl.h
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#ifndef TAG_CLOUD_IMPL_H
#define TAG_CLOUD_IMPL_H

#include "oslib/font.h"
#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/datastruct/bitvec.h"
#include "appengine/datastruct/atom.h"
#include "appengine/base/errors.h"

#include "appengine/gadgets/tag-cloud.h"

typedef struct tag_cloud_entry
{
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
  int              gaplength; /* length of the gap string in font units */

  tag_cloud_dim  *dims;
  int              allocated;
}
tag_cloud_dims;

/* Cached lengths of highlighted tags. */
typedef struct tag_cloud_lengths
{
  int              widest;
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
  tag_cloud_FLAG_NEW_DATA       = (1 << 0),
  tag_cloud_FLAG_NEW_HIGHLIGHTS = (1 << 1),
  tag_cloud_FLAG_NEW_HOVER      = (1 << 2),
  tag_cloud_FLAG_NEW_SORT       = (1 << 3),
  tag_cloud_FLAG_NEW_DISPLAY    = (1 << 4),
  tag_cloud_FLAG_NEW_ALL        = 0x1f,
  tag_cloud_FLAG_SHADED         = (1 << 5),
  tag_cloud_FLAG_SORT_SEL_FIRST = (1 << 6),
  tag_cloud_FLAG_TOOLBAR        = (1 << 7),
  tag_cloud_FLAG_TOOLBAR_NOT_EVER = (1 << 8),
  tag_cloud_FLAG_LAYOUT_PREPED  = (1 << 9),
};

typedef unsigned int tag_cloud_flags;

struct tag_cloud
{
  tag_cloud_config         config;

  tag_cloud_flags          flags;

  wimp_w                    main_w; /* cloned per instance */
  wimp_w                    toolbar_w;
  wimp_menu                *main_m; // should not need to be per-instance

  tag_cloud_newtagfn      *newtag;
  tag_cloud_deletetagfn   *deletetag;
  tag_cloud_renametagfn   *renametag;
  tag_cloud_tagfn         *tag;
  tag_cloud_tagfn         *detag;
  tag_cloud_tagfilefn     *tagfile;
  tag_cloud_tagfilefn     *detagfile;
  tag_cloud_eventfn       *event;
  void                     *arg;

  tag_cloud_key_handler_fn *key_handler;
  void                     *key_handler_arg;

  atom_set_t               *dict;

  tag_cloud_entry         *entries;
  int                       e_used;
  int                       e_allocated;

  atom_t                   *sorted; /* of length e_used */

  int                       sort_type;
  int                       order_type;
  int                       scaletab[5];
  int                       display_type;

  tag_cloud_scale_data     scale;

  tag_cloud_layout_data    layout;

  tag_cloud_highlight_data highlight;

  tag_cloud_hover_data     hover;

  int                       menued_tag_index;

  struct
  {
    int                     last_sort_type;
    int                     last_order_type;
  }
  sort;
};

void tag_cloud_internal_set_handlers(int reg, tag_cloud *tc);

/* layout.c */
error tag_cloud_layout_prepare(tag_cloud *tc);
error tag_cloud_layout(tag_cloud *tc, int width);
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
void tag_cloud_schedule_redraw(tag_cloud *tc);

/* open.c */
void tag_cloud_post_open(tag_cloud *tc, wimp_open *open);
void tag_cloud_post_reopen(tag_cloud *tc, wimp_open *open);

/* highlight.c */
int tag_cloud_is_highlighted(tag_cloud *tc, int index);

/* toolbar.c */
void tag_cloud_attach_toolbar(tag_cloud *tc);
void tag_cloud_detach_toolbar(tag_cloud *tc);
void tag_cloud_toggle_toolbar(tag_cloud *tc);

#endif /* TAG_CLOUD_IMPL_H */
