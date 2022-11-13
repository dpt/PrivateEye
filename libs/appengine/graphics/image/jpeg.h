/* --------------------------------------------------------------------------
 *    Name: jpeg.h
 * Purpose: JPEG module header
 * ----------------------------------------------------------------------- */

#ifndef IMAGE_JPEG_H
#define IMAGE_JPEG_H

#include "appengine/graphics/image.h"

/* jpeg-meta.c */
extern int jpeg_get_meta(image_t *image, ntree_t **data);
extern int jpeg_meta_available(image_t *image);

/* jpeg.c */
extern void jpeg_export_methods(const image_choices *choices, image_t *image);

/* lifted from jdmarker.c */
typedef enum        /* JPEG marker codes */
{
  M_SOF0  = 0xc0,
  M_SOF1  = 0xc1,
  M_SOF2  = 0xc2,
  M_SOF3  = 0xc3,

  M_SOF5  = 0xc5,
  M_SOF6  = 0xc6,
  M_SOF7  = 0xc7,

  M_JPG   = 0xc8,
  M_SOF9  = 0xc9,
  M_SOF10 = 0xca,
  M_SOF11 = 0xcb,

  M_SOF13 = 0xcd,
  M_SOF14 = 0xce,
  M_SOF15 = 0xcf,

  M_DHT   = 0xc4,

  M_DAC   = 0xcc,

  M_RST0  = 0xd0,
  M_RST1  = 0xd1,
  M_RST2  = 0xd2,
  M_RST3  = 0xd3,
  M_RST4  = 0xd4,
  M_RST5  = 0xd5,
  M_RST6  = 0xd6,
  M_RST7  = 0xd7,

  M_SOI   = 0xd8,
  M_EOI   = 0xd9,
  M_SOS   = 0xda,
  M_DQT   = 0xdb,
  M_DNL   = 0xdc,
  M_DRI   = 0xdd,
  M_DHP   = 0xde,
  M_EXP   = 0xdf,

  M_APP0  = 0xe0,
  M_APP1  = 0xe1,
  M_APP2  = 0xe2,
  M_APP3  = 0xe3,
  M_APP4  = 0xe4,
  M_APP5  = 0xe5,
  M_APP6  = 0xe6,
  M_APP7  = 0xe7,
  M_APP8  = 0xe8,
  M_APP9  = 0xe9,
  M_APP10 = 0xea,
  M_APP11 = 0xeb,
  M_APP12 = 0xec,
  M_APP13 = 0xed,
  M_APP14 = 0xee,
  M_APP15 = 0xef,

  M_JPG0  = 0xf0,
  M_JPG13 = 0xfd,
  M_COM   = 0xfe,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;

/* jpeg-utils.c */

void jpeg_find(const unsigned char  *jpeg_data,
               int                   file_size,
               JPEG_MARKER           marker,
               const unsigned char **location,
               int                  *length,
               int                  *offset);

typedef enum jpeg_colourspace
{
  jpeg_COLOURSPACE_GREYSCALE,
  jpeg_COLOURSPACE_RGB,
  jpeg_COLOURSPACE_YCBCR,
  jpeg_COLOURSPACE_CMYK,
  jpeg_COLOURSPACE_YCCK,
  jpeg_COLOURSPACE_UNKNOWN
}
jpeg_colourspace_t;

#define jpeg_FLAG_JFIF      (1u <<  0)
#define jpeg_FLAG_EXIF      (1u <<  1)
#define jpeg_FLAG_ADOBE     (1u <<  2)
#define jpeg_FLAG_BASELINE  (1u <<  4)
#define jpeg_FLAG_EXTSEQ    (1u <<  5)
#define jpeg_FLAG_PRGRSSVE  (1u <<  6)
#define jpeg_FLAG_ARITH     (1u <<  9) /* otherwise Huffman */
#define jpeg_FLAG_TRUNCATED (1u << 30)

typedef unsigned int jpeg_flags_t;

typedef struct jpeg_info
{
  jpeg_flags_t       flags;
  jpeg_colourspace_t colourspace;
}
jpeg_info_t;

int jpeg_get_info(const unsigned char *jpeg_data,
                  int                  file_size,
                  jpeg_info_t         *info);

/* Returns nonzero if the JPEG originating the given info structure is
 * directly renderable with the current OS. */
int jpeg_supported(const jpeg_info_t *info);

#endif /* IMAGE_JPEG_H */
