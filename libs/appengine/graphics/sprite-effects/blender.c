/* --------------------------------------------------------------------------
 *    Name: blender.c
 * Purpose: Alpha blending between two bitmaps
 * ----------------------------------------------------------------------- */

#include "kernel.h"
#include "swis.h"

#include <stdio.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "oslib/osspriteop.h"

#include "appengine/base/errors.h"
#include "appengine/vdu/sprite.h"

#include "appengine/graphics/sprite-effects.h"

/* ----------------------------------------------------------------------- */

enum
{
  Shift = 16
};

/* ----------------------------------------------------------------------- */

#define CLAMP(v,max) do { \
                       if (v < 0) v = 0; else if (v > max) v = max; \
                     } while (0)

/* ----------------------------------------------------------------------- */

static void make_lut_555(int alpha, int offset, blender *args)
{
  int          i;
  int          calpha; /* complement of alpha */
  blender_lut *lutA = &args->lutA[0];
  blender_lut *lutB = &args->lutB[0];

  const int middle = (1 << Shift);

  offset = offset * 32 / 256;

  if (alpha < middle)
  {
    args->src_to_use = args->deg;
    args->deg_to_use = args->src;
    alpha = middle - alpha;
  }
  else
  {
    args->src_to_use = args->src;
    args->deg_to_use = args->deg;
  }

  calpha = (1 << Shift) - alpha;

  for (i = 0; i < 32; i++)
  {
    *lutA++ = offset + ((calpha * i) >> Shift);
    *lutB++ = offset + (( alpha * i) >> Shift);
  }
}

static void make_lut_888(int alpha, int offset, blender *args)
{
  int          i;
  int          calpha; /* complement of alpha */
  blender_lut *lutA = &args->lutA[0];
  blender_lut *lutB = &args->lutB[0];

  const int middle = (1 << Shift);

  if (alpha < middle)
  {
    args->src_to_use = args->deg;
    args->deg_to_use = args->src;
    alpha = middle - alpha;
  }
  else
  {
    args->src_to_use = args->src;
    args->deg_to_use = args->deg;
  }

  /*printf("%csrc=%p dst=%p alpha=%x  ", 13, args->src_to_use,
                                           args->deg_to_use,
                                           alpha);*/

  calpha = (1 << Shift) - alpha;

  for (i = 0; i < 256; i++)
  {
    *lutA++ = offset + ((calpha * i) >> Shift);
    *lutB++ = offset + (( alpha * i) >> Shift);
  }
}

/* ----------------------------------------------------------------------- */

#define BLEND(A,B) (lutA[A] + lutB[B])

/* ----------------------------------------------------------------------- */

static void blend_555(blender *args)
{
  int                xy;
  int               *dst;
  const int         *src;
  const int         *deg;
  const blender_lut *lutA = &args->lutA[0];
  const blender_lut *lutB = &args->lutB[0];

  dst = args->dst;
  src = args->src_to_use;
  deg = args->deg_to_use;

  for (xy = args->width * args->height; xy--; )
  {
    unsigned int pa;
    unsigned int pb;
    int          a;
    int          b;
    int          t;
    unsigned int pc;

    /* process two pixels at a time */

    pa = *src++;
    pb = *deg++;

    a = (pa >> 0) & 0x1f;
    b = (pb >> 0) & 0x1f;
    t = BLEND(a,b); CLAMP(t,0x1f);
    pc = t;

    a = (pa >> 5) & 0x1f;
    b = (pb >> 5) & 0x1f;
    t = BLEND(a,b); CLAMP(t,0x1f);
    pc |= t << 5;

    a = (pa >> 10) & 0x1f;
    b = (pb >> 10) & 0x1f;
    t = BLEND(a,b); CLAMP(t,0x1f);
    pc |= t << 10;

    /* ignoring alpha bit for now */

    a = (pa >> 16) & 0x1f;
    b = (pb >> 16) & 0x1f;
    t = BLEND(a,b); CLAMP(t,0x1f);
    pc |= t << 16;

    a = (pa >> 21) & 0x1f;
    b = (pb >> 21) & 0x1f;
    t = BLEND(a,b); CLAMP(t,0x1f);
    pc |= t << 21;

    a = (pa >> 26) & 0x1f;
    b = (pb >> 26) & 0x1f;
    t = BLEND(a,b); CLAMP(t,0x1f);
    pc |= t << 26;

    /* ignoring alpha bit for now */

    *dst++ = pc;
  }
}

static void blend_888(blender *args)
{
  int                xy;
  int               *dst;
  const int         *src;
  const int         *deg;
  const blender_lut *lutA = &args->lutA[0];
  const blender_lut *lutB = &args->lutB[0];

  dst = args->dst;
  src = args->src_to_use;
  deg = args->deg_to_use;

  for (xy = args->width * args->height; xy--; )
  {
    unsigned int pa;
    unsigned int pb;
    int          a;
    int          b;
    int          t;
    unsigned int pc;

    /* 0xAABBGGRR */

    pa = *src++;
    pb = *deg++;

    /* red */

    a = pa & 0xff;
    b = pb & 0xff;
    t = BLEND(a,b); CLAMP(t,0xff);
    pc = t;

    /* green */

    a = (pa >> 8) & 0xff;
    b = (pb >> 8) & 0xff;
    t = BLEND(a,b); CLAMP(t,0xff);
    pc |= t << 8;

    /* blue */

    a = (pa >> 16) & 0xff;
    b = (pb >> 16) & 0xff;
    t = BLEND(a,b); CLAMP(t,0xff);
    pc |= t << 16;

    /* alpha */

    a = (pa >> 24) & 0xff;
    b = (pb >> 24) & 0xff;
    t = BLEND(a,b); CLAMP(t,0xff);
    pc |= t << 24;

    *dst++ = pc;
  }
}

/* ----------------------------------------------------------------------- */

error blender_create(blender *b, osspriteop_area *area)
{
  osspriteop_header   *header;
  osspriteop_mode_word mode;
  int                  log2bpp;

  header = sprite_select(area, 0);

  sprite_info(area,
              header,
             &b->width, &b->height,
              NULL,
 (os_mode *) &mode,
             &log2bpp);

  switch (log2bpp)
  {
  case 4:
    b->make_lut = make_lut_555;
    b->blend    = blend_555;
    break;

  case 5:
    b->make_lut = make_lut_888;
    b->blend    = blend_888;
    break;

  default:
    return error_SPRITEFX_UNSUPP_EFFECT;
  }

  return error_OK;
}
