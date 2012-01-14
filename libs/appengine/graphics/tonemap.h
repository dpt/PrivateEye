/* --------------------------------------------------------------------------
 *    Name: tonemap.h
 * Purpose: Tone maps
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_TONEMAP_H
#define APPENGINE_TONEMAP_H

#include "oslib/os.h"

#define T tonemap

typedef struct T T;

T *tonemap_create(void);

void tonemap_destroy(T *map);

void tonemap_reset(T *map);

T *tonemap_copy(const T *map);


typedef unsigned int tonemap_channels;

#define tonemap_CHANNEL_RED   (  1u << 0)
#define tonemap_CHANNEL_GREEN (  1u << 1)
#define tonemap_CHANNEL_BLUE  (  1u << 2)
#define tonemap_CHANNEL_ALPHA (  1u << 3)

#define tonemap_CHANNEL_RGB   (0x7u << 0)
#define tonemap_CHANNEL_RGBA  (0xfu << 0)


typedef unsigned int tonemap_flags;

#define tonemap_FLAG_INVERT   (  1u << 0)
#define tonemap_FLAG_REFLECT  (  1u << 1)


/* Typical ranges:
 * gamma      - 10..290
 * brightness -  0..200
 * contrast   -  0..200
 * midd       -  0..100
 * bias       -  0..100
 * gain       -  0..100
 */
typedef struct tonemap_spec
{
  tonemap_flags flags;
  int           gamma;
  int           brightness;
  int           contrast;
  int           midd;
  int           bias;
  int           gain;
}
tonemap_spec;

void tonemap_set(T               *map,
                 tonemap_channels channels,
           const tonemap_spec    *spec);

error tonemap_draw(T   *map,
                   int  x,
                   int  y);

void tonemap_draw_set_stroke_width(T  *map,
                                   int width);

void tonemap_get_corrections(T                          *map,
                             const os_correction_table **red,
                             const os_correction_table **green,
                             const os_correction_table **blue,
                             const os_correction_table **alpha);

void tonemap_get_values(T               *map,
                        tonemap_channels channels,
                        tonemap_spec    *spec);

#undef T

#endif /* APPENGINE_TONEMAP_H */
