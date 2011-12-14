/* --------------------------------------------------------------------------
 *    Name: image.h
 * Purpose: Image block handling header
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_IMAGE_H
#define APPENGINE_IMAGE_H

#include <stdlib.h>

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/wimp.h"

#include "appengine/datastruct/list.h"
#include "appengine/vdu/sprite.h"
#include "appengine/datastruct/ntree.h"

#define T image_t

/* ----------------------------------------------------------------------- */

/* Size of a digest. */
#define image_DIGESTSZ 16

/* ----------------------------------------------------------------------- */

typedef int image_jpeg_cleaning;
enum
{
  image_JPEG_CLEANING_IF_REQUIRED = 0,
  image_JPEG_CLEANING_ALWAYS      = 1
};

typedef struct
{
  struct
  {
    int               border;
  }
  artworks;

  struct
  {
    int               border;
  }
  drawfile;

  struct
  {
    image_jpeg_cleaning cleaning;
    osbool            sprite;   /* convert to sprite */
    osbool            trim;     /* trim when rotating */
  }
  jpeg;

  struct
  {
    osbool            ignore_gamma;
    osbool            ignore_scale;
    osbool            ignore_transparency;
  }
  png;
}
image_choices;

/* ----------------------------------------------------------------------- */

/* FIXME: We might need to differentiate between source and display flags
 *        ie. source vector converted to sprite. */

enum
{
  image_FLAG_VECTOR     = 1 << 0,
  image_FLAG_COLOUR     = 1 << 1,
  image_FLAG_HAS_MASK   = 1 << 2,  /* has sprite-style binary mask */
  image_FLAG_EDITING    = 1 << 3,
  image_FLAG_MODIFIED   = 1 << 4,
  image_FLAG_CAN_HIST   = 1 << 5,  /* can obtain histogram */
  image_FLAG_CAN_ROT    = 1 << 6,  /* can rotate */
  image_FLAG_HAS_META   = 1 << 7,  /* has metadata */
  image_FLAG_CAN_SPR    = 1 << 8,  /* can convert to sprite */
  image_FLAG_HAS_ALPHA  = 1 << 9,  /* has alpha mask */
  image_FLAG_HAS_DIGEST = 1 << 10, /* has digest computed */
};

typedef unsigned int image_flags;


typedef struct T T;


struct image_methods
{
  int (*load)     (image_choices *choices,
                   T             *image);

  int (*save)     (image_choices *choices,
                   T             *image,
                   const char    *file_name);

  int (*unload)   (T             *image);

  int (*histogram)(T             *image);

  int (*rotate)   (image_choices *choices,
                   T             *image,
                   int            angle);

  int (*get_meta) (T             *image,
                   ntree_t      **data);

  int (*to_spr)   (T             *image);
};

typedef struct image_methods image_methods;


typedef struct image_attrs image_attrs;

struct image_attrs
{
  union
  {
    struct
    {
      int    width, height;
      int    xeig, yeig;
      int    xdpi, ydpi;
      int    bpp;
    }
    bm; /* bitmaps only */

    struct
    {
      os_box box;
    }
    vc; /* vectors only */
  }
  dims;

  bits       load, exec; /* only used in 'source' */
  size_t     file_size;  /* bytes */
  bits       file_type;
};


struct T
{
  list_t        list;

  image_methods methods;

  image_flags   flags;

  void         *image;

  char          file_name[256]; /* careful now */
  unsigned char digest[image_DIGESTSZ];

  int           refcount;

  image_attrs   display;
  image_attrs   source;

  struct
  {
    int         min, max;
  }
  scale;

  union
  {
    struct
    {
      os_mode   mode;
      int       index;
    }
    sprite;

    struct
    {
      void     *workspace;
    }
    artworks;
  }
  details;

  sprite_histograms *hists;

  os_colour          background_colour;
};

/* ----------------------------------------------------------------------- */

/* Creates an empty image. */
T *image_create(void);

/* Creates and loads the specified filename. */
T *image_create_from_file(image_choices *choices,
                          const char    *file_name,
                          bits           file_type);

void image_destroy(T *image);

/* ----------------------------------------------------------------------- */

void image_addref(T *image);
void image_deleteref(T *image);

/* ----------------------------------------------------------------------- */

int image_start_editing(T *image);
void image_stop_editing(T *image);
osbool image_is_editing(const T *image);

void image_about_to_modify(T *image);

enum
{
  image_MODIFIED_DATA   = 1 << 0,
  image_MODIFIED_FORMAT = 1 << 1,
};

typedef unsigned int image_modified_flags;

void image_modified(T *image, image_modified_flags flags);

/* ----------------------------------------------------------------------- */

void image_select(T *image, int index);
void image_preview(T *image);

/* ----------------------------------------------------------------------- */

void image_hide(T *i);
void image_reveal(T *i);

/* ----------------------------------------------------------------------- */

void image_focus(T *i);
void image_defocus(T *i);

/* ----------------------------------------------------------------------- */

#define png_FILE_TYPE 0xb60
#define gif_FILE_TYPE 0x695
#define artworks_FILE_TYPE 0xd94

osbool image_is_loadable(bits file_type);

/* ----------------------------------------------------------------------- */

int image_recognise(wimp_message *message);

/* ----------------------------------------------------------------------- */

typedef void (image_map_callback)(T *, void *arg);

void image_map(image_map_callback *fn, void *arg);

/* ----------------------------------------------------------------------- */

int image_get_count(void);

/* ----------------------------------------------------------------------- */

/* Destroys the tree of metadata returned by the get_meta entry point. */
void image_destroy_metadata(ntree_t *metadata);

/* ----------------------------------------------------------------------- */

error image_get_digest(T *image, unsigned char digest[image_DIGESTSZ]);

#undef T

#endif /* APPENGINE_IMAGE_H */
