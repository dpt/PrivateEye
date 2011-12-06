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

typedef struct tag_cloud__entry
{
  int              count;
}
tag_cloud__entry;

typedef struct tag_cloud__scale_data
{
  int              min;
  int              scale;
}
tag_cloud__scale_data;

typedef struct tag_cloud__boxes
{
  os_box          *boxes; /* array of boxes, one per tag */
  int              allocated;
}
tag_cloud__boxes;

typedef struct tag_cloud__dim
{
  int              length[2]; /* length for regular, bold */
  int              scale;
}
tag_cloud__dim;

typedef struct tag_cloud__dims
{
  int              gaplength; /* length of the gap string in font units */

  tag_cloud__dim  *dims;
  int              allocated;
}
tag_cloud__dims;

/* Cached lengths of highlighted tags. */
typedef struct tag_cloud__lengths
{
  int              widest;
  int             *length;
  int              allocated;
}
tag_cloud__lengths;

typedef struct tag_cloud__paintstring
{
  char            *string;
  int              allocated;
  int              used;
}
tag_cloud__paintstring;

typedef struct tag_cloud__layout_data
{
  int              width;     /* OS units */
  int              height;    /* OS units */

  int              font_size; /* OS units */
  int              leading;   /* OS units */
  int              padding;   /* OS units */

  int              descender; /* font units */

  font_f           font;      /* regular font */
  font_f           bold_font; /* bold font */

  tag_cloud__paintstring paintstring;
  tag_cloud__dims        dims;
  tag_cloud__lengths     lengths;
  tag_cloud__boxes       boxes;
}
tag_cloud__layout_data;

typedef bitvec_t *tag_cloud__highlight_data;

typedef struct tag_cloud__hover_data
{
  atom_t           index; /* tag to underline */
  int              last_index;
  const char      *pointer_shape_name; /* or NULL if none */
}
tag_cloud__hover_data;

enum
{
  tag_cloud__FLAG_NEW_DATA       = (1 << 0),
  tag_cloud__FLAG_NEW_HIGHLIGHTS = (1 << 1),
  tag_cloud__FLAG_NEW_HOVER      = (1 << 2),
  tag_cloud__FLAG_NEW_SORT       = (1 << 3),
  tag_cloud__FLAG_NEW_DISPLAY    = (1 << 4),
  tag_cloud__FLAG_NEW_ALL        = 0x1f,
  tag_cloud__FLAG_SHADED         = (1 << 5),
  tag_cloud__FLAG_SORT_SEL_FIRST = (1 << 6),
  tag_cloud__FLAG_TOOLBAR        = (1 << 7),
  tag_cloud__FLAG_TOOLBAR_NOT_EVER = (1 << 8),
  tag_cloud__FLAG_LAYOUT_PREPED  = (1 << 9),
};

typedef unsigned int tag_cloud__flags;

struct tag_cloud
{
  tag_cloud__config         config;

  tag_cloud__flags          flags;

  wimp_w                    main_w; /* cloned per instance */
  wimp_w                    toolbar_w;
  wimp_menu                *main_m; // should not need to be per-instance

  tag_cloud__newtagfn      *newtag;
  tag_cloud__deletetagfn   *deletetag;
  tag_cloud__renametagfn   *renametag;
  tag_cloud__tagfn         *tag;
  tag_cloud__tagfn         *detag;
  tag_cloud__tagfilefn     *tagfile;
  tag_cloud__tagfilefn     *detagfile;
  tag_cloud__eventfn       *event;
  void                     *arg;

  tag_cloud__key_handler_fn *key_handler;
  void                     *key_handler_arg;

  atom_set_t               *dict;

  tag_cloud__entry         *entries;
  int                       e_used;
  int                       e_allocated;

  atom_t                   *sorted; /* of length e_used */

  int                       sort_type;
  int                       order_type;
  int                       scaletab[5];
  int                       display_type;

  tag_cloud__scale_data     scale;

  tag_cloud__layout_data    layout;

  tag_cloud__highlight_data highlight;

  tag_cloud__hover_data     hover;

  int                       menued_tag_index;

  struct
  {
    int                     last_sort_type;
    int                     last_order_type;
  }
  sort;
};

void tag_cloud__internal_set_handlers(int reg, tag_cloud *tc);

/* layout.c */
error tag_cloud__layout_prepare(tag_cloud *tc);
error tag_cloud__layout(tag_cloud *tc, int width);
void tag_cloud__layout_reset(tag_cloud *tc);
/* Discard cached metrics. */
void tag_cloud__layout_discard(tag_cloud *tc);
int tag_cloud__hit(tag_cloud *tc, int x, int y);

/* init.c */
wimp_w tag_cloud__get_main_window(void);
wimp_w tag_cloud__get_toolbar_window(void);
dialogue_t *tag_cloud__get_newtag_dialogue(void);
dialogue_t *tag_cloud__get_renametag_dialogue(void);
dialogue_t *tag_cloud__get_taginfo_dialogue(void);

/* hover.c */
void tag_cloud__hover_init(tag_cloud *tc);
void tag_cloud__hover(tag_cloud *tc, int x, int y);
void tag_cloud__restore_pointer_shape(tag_cloud *tc);
void tag_cloud__hover_toggle(tag_cloud *tc);

/* redraw.c */
void tag_cloud__schedule_redraw(tag_cloud *tc);

/* open.c */
void tag_cloud__post_open(tag_cloud *tc, wimp_open *open);
void tag_cloud__post_reopen(tag_cloud *tc, wimp_open *open);

/* highlight.c */
int tag_cloud__is_highlighted(tag_cloud *tc, int index);

/* toolbar.c */
void tag_cloud__attach_toolbar(tag_cloud *tc);
void tag_cloud__detach_toolbar(tag_cloud *tc);
void tag_cloud__toggle_toolbar(tag_cloud *tc);

#endif /* TAG_CLOUD_IMPL_H */
