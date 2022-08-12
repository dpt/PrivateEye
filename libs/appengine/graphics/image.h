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

#include "datastruct/list.h"
#include "datastruct/ntree.h"

#include "appengine/vdu/sprite.h"

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
    int                 border;
  }
  artworks;

  struct
  {
    int                 border;
  }
  drawfile;

  struct
  {
    image_jpeg_cleaning cleaning;
    osbool              sprite;   /* convert to sprite */
    osbool              trim;     /* trim when rotating */
  }
  jpeg;

  struct
  {
    osbool              ignore_gamma;
    osbool              ignore_scale;
    osbool              ignore_transparency;
  }
  png;
}
image_choices;

/* ----------------------------------------------------------------------- */

/* FIXME: We might need to differentiate between source and display flags
 *        ie. source vector converted to sprite. */

enum
{
  image_FLAG_VECTOR     = 1 << 0,  /* is a vector format image */
  image_FLAG_COLOUR     = 1 << 1,  /* is a colour image (otherwise mono) */
  image_FLAG_HAS_MASK   = 1 << 2,  /* has sprite-style binary mask */
  image_FLAG_EDITING    = 1 << 3,  /* is currently being edited */
  image_FLAG_MODIFIED   = 1 << 4,  /* is modified or unsaved (reset on saves) */
  image_FLAG_CAN_HIST   = 1 << 5,  /* can obtain histogram */
  image_FLAG_CAN_ROT    = 1 << 6,  /* can rotate */
  image_FLAG_HAS_META   = 1 << 7,  /* has metadata */
  image_FLAG_CAN_SPR    = 1 << 8,  /* can convert to sprite */
  image_FLAG_HAS_ALPHA  = 1 << 9,  /* has alpha mask */
  image_FLAG_HAS_DIGEST = 1 << 10, /* has a digest computed */
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


/* image_attrs holds the essential attributes of an image. */
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


/* image_info holds the nice-to-have informational attributes of an image. */
typedef struct image_info image_info;

typedef enum
{
  image_INFO_BPC,         /* Byte - Bits Per Component - e.g. 8 bpc */
  image_INFO_COLOURSPACE, /* String - Name of colourspace - e.g. "YCCK" */
  image_INFO_FORMAT,      /* String - Format details - e.g. "JFIF+Exif, Baseline" for a JPEG */
  image_INFO_NCOMPONENTS, /* Byte - Number of Components - e.g. 3 components */
  image_INFO_ORDERING,    /* TBD - Interlacing - PNG/GIF */
  image_INFO_PALETTE,     /* TBD - Number of Palette Entries */
}
image_info_key;

struct image_info
{
  image_info_key  key;
  unsigned char  *data;
};

typedef struct imageinfo imageinfo;

struct imageinfo
{
  image_info    *entries;
  int            entriesused;
  int            entriesallocated;

  unsigned char *data;
  int            dataused;
  int            dataallocated;
};


struct T
{
  list_t        list;

  image_methods methods;

  image_flags   flags;

  void         *image;

  char          file_name[256]; /* NUL for no file name */
  unsigned char digest[image_DIGESTSZ];

  imageinfo     info;

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
                          bits           file_type,
                          osbool         unsafe);

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

/** Call this when an image was saved to disc. */
void image_saved(T *i);

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

typedef void (image_map_callback)(T *, void *opaque);

void image_map(image_map_callback *fn, void *opaque);

/* ----------------------------------------------------------------------- */

int image_get_count(void);

/* ----------------------------------------------------------------------- */

/* Destroys the tree of metadata returned by the get_meta entry point. */
void image_destroy_metadata(ntree_t *metadata);

/* ----------------------------------------------------------------------- */

result_t image_get_digest(T *image, unsigned char digest[image_DIGESTSZ]);

/* ----------------------------------------------------------------------- */

result_t image_set_info(image_t        *image,
                        image_info_key  key,
                        const void     *data);

int image_get_info(image_t         *image,
                   image_info_key   key,
                   void           **data); // make const

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_IMAGE_H */
