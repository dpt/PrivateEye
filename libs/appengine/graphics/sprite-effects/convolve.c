/* --------------------------------------------------------------------------
 *    Name: convolve.c
 * Purpose: Convolves images
 * ----------------------------------------------------------------------- */

/* Based on "Fast Convolution with Packed Lookup Tables"
 * by George Wolberg and Henry Massalin, in "Graphics Gems IV". */

#include <assert.h>
#include <stdlib.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"
#include "appengine/types.h"
#include "appengine/vdu/sprite.h"

#include "convolve.h"

/* ----------------------------------------------------------------------- */

#define MASK 0x3FF
#define ROUNDD 1
#define PACK(A,B,C) (((A) << 20) + ((B) << 10) + (C))
#define INT(A) ((int) ((A) * 262144.0f + 32768.0f) >> 16)

/* ----------------------------------------------------------------------- */

typedef unsigned char Pixel8;
typedef unsigned int Pixel888;

/* ----------------------------------------------------------------------- */

#define MAXLUTS 8

struct convolve_lut
{
  int lut[MAXLUTS][256];
  int bias;
  int stages;
};

/* ----------------------------------------------------------------------- */

error convolve_init(const float *kernel, int n, convolve_lut **new_lut)
{
  convolve_lut *lut;
  int           s;
  int           k;

  lut = malloc(sizeof(*lut));
  if (lut == NULL)
    return error_OOM;

  lut->bias = 0;

  for (k = s = 0; k < n; s++)
  {
    float k1, k2, k3;
    int   b1, b2, b3;
    int  *tab;
    int   i;

    assert(s < MAXLUTS);

    k1 = (k < n) ? kernel[k++] : 0.0f;
    k2 = (k < n) ? kernel[k++] : 0.0f;
    k3 = (k < n) ? kernel[k++] : 0.0f;
    if (k <= 3)
      k1 *= 0.5f;

    b1 = b2 = b3 = 0;
    if (k1 < 0) b1 = (int) (-k1 * 1024.0f);
    if (k2 < 0) b2 = (int) (-k2 * 1024.0f);
    if (k3 < 0) b3 = (int) (-k3 * 1024.0f);

    lut->bias += 2 * (b1 + b2 + b3);

    tab = lut->lut[s];

    for (i = 0; i < 256; i++)
    {
      tab[i] = PACK(INT((float) i * k3) + b3,
                    INT((float) i * k2) + b2 + ROUNDD,
                    INT((float) i * k1) + b1);
    }
  }

  lut->stages = s;

  *new_lut = lut;

  return error_OK;
}

void convolve_destroy(convolve_lut *lut)
{
  free(lut);
}

/* ----------------------------------------------------------------------- */

static error convolve_8(const convolve_lut *lut,
                        const Pixel8       *src,
                        Pixel8             *dst,
                        int                 len,
                        int                 stride,
                        Pixel8             *buf)
{
  int             padlen;
  const Pixel8   *p1;
  Pixel8         *p2;
  int             x;
  const Pixel8   *ip;
  Pixel8         *op;
  int             bias;
  const int      *lut0;

  int fwd0;
  //int fwd1;
  //int fwd2;
  int rev0;
  //int rev1;
  //int rev2;
  int val;

  /* buffer and pad the input */

  padlen = 3 * (lut->stages) - 1;
  p1 = src;
  p2 = buf;
  for (x = 0; x < padlen; x++)
    *p2++ = *p1;
  for (x = 0; x < len; x++)
  {
    *p2++ = *p1;
    p1 += stride;
  }
  p1 -= stride;
  for (x = 0; x < padlen; x++)
    *p2++ = *p1;


  ip = buf;
  op = dst;

  bias = lut->bias;

  switch (lut->stages)
  {
  case 1: /* 5-pt kernel */
    lut0 = lut->lut[0];

    ip += 2; /* ip[0] is centre pixel */

    fwd0 = (lut0[ip[-2]] >> 10) + lut0[ip[-1]];
    rev0 = (lut0[ip[ 0]] << 10) + lut0[ip[ 1]];

    while (len--)
    {
      fwd0 = (fwd0 >> 10) + lut0[ip[0]];
      rev0 = (rev0 << 10) + lut0[ip[2]];
      val = ((fwd0 & MASK) + ((rev0 >> 20) & MASK) - bias) >> 2;
      *op = CLAMP(val, 0, 255);
      ip++;
      op += stride;
    }

    break;

  case 2: /* 11-pt kernel */
  case 3: /* 17-pt kernel */
    return error_SPRITEFX_UNSUPP_EFFECT;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

typedef void convolve_888_fn(const convolve_lut *lut,
                             const Pixel888     *ip,
                             Pixel888           *op,
                             int                 len,
                             int                 bias,
                             int                 stride);

#define RED(PX)   ((PX >> 0)  & 0xff)
#define GREEN(PX) ((PX >> 8)  & 0xff)
#define BLUE(PX)  ((PX >> 16) & 0xff)

#define FUSE(R,G,B) ((R << 0) | (G << 8) | (B << 16))

static void convolve_888_1stage(const convolve_lut *lut,
                                const Pixel888     *ip,
                                Pixel888           *op,
                                int                 len,
                                int                 bias,
                                int                 stride)
{
  const int      *lut0;
  int             fwd0_r;
  int             rev0_r;
  int             fwd0_g;
  int             rev0_g;
  int             fwd0_b;
  int             rev0_b;

  lut0 = lut->lut[0];

  ip += 2; /* ip[0] is centre pixel */

  /* See the g8 version of the code for a simpler equivalent */

  fwd0_r = (lut0[RED  (ip[-2])] >> 10) + lut0[RED  (ip[-1])];
  fwd0_g = (lut0[GREEN(ip[-2])] >> 10) + lut0[GREEN(ip[-1])];
  fwd0_b = (lut0[BLUE (ip[-2])] >> 10) + lut0[BLUE (ip[-1])];

  rev0_r = (lut0[RED  (ip[ 0])] << 10) + lut0[RED  (ip[ 1])];
  rev0_g = (lut0[GREEN(ip[ 0])] << 10) + lut0[GREEN(ip[ 1])];
  rev0_b = (lut0[BLUE (ip[ 0])] << 10) + lut0[BLUE (ip[ 1])];

  while (len--)
  {
    int val_r, val_g, val_b;

    fwd0_r = (fwd0_r >> 10) + lut0[RED  (ip[0])];
    fwd0_g = (fwd0_g >> 10) + lut0[GREEN(ip[0])];
    fwd0_b = (fwd0_b >> 10) + lut0[BLUE (ip[0])];

    rev0_r = (rev0_r << 10) + lut0[RED  (ip[2])];
    rev0_g = (rev0_g << 10) + lut0[GREEN(ip[2])];
    rev0_b = (rev0_b << 10) + lut0[BLUE (ip[2])];

    val_r = ((fwd0_r & MASK) + ((rev0_r >> 20) & MASK) - bias) >> 2;
    val_r = CLAMP(val_r, 0, 255);

    val_g = ((fwd0_g & MASK) + ((rev0_g >> 20) & MASK) - bias) >> 2;
    val_g = CLAMP(val_g, 0, 255);

    val_b = ((fwd0_b & MASK) + ((rev0_b >> 20) & MASK) - bias) >> 2;
    val_b = CLAMP(val_b, 0, 255);

    *op = FUSE(val_r, val_g, val_b) | (ip[0] & 0xff000000);

    ip++;

    op += stride;
  }
}

static void convolve_888_2stage(const convolve_lut *lut,
                                const Pixel888     *ip,
                                Pixel888           *op,
                                int                 len,
                                int                 bias,
                                int                 stride)
{
  const int      *lut0;
  const int      *lut1;
  int             fwd0_r, fwd1_r;
  int             rev0_r, rev1_r;
  int             fwd0_g, fwd1_g;
  int             rev0_g, rev1_g;
  int             fwd0_b, fwd1_b;
  int             rev0_b, rev1_b;

  lut0 = lut->lut[0];
  lut1 = lut->lut[1];

  ip += 5; /* ip[0] is centre pixel */

  fwd0_r = (lut0[RED  (ip[-2])] >> 10) + lut0[RED  (ip[-1])];
  fwd0_g = (lut0[GREEN(ip[-2])] >> 10) + lut0[GREEN(ip[-1])];
  fwd0_b = (lut0[BLUE (ip[-2])] >> 10) + lut0[BLUE (ip[-1])];

  rev0_r = (lut0[RED  (ip[ 0])] << 10) + lut0[RED  (ip[ 1])];
  rev0_g = (lut0[GREEN(ip[ 0])] << 10) + lut0[GREEN(ip[ 1])];
  rev0_b = (lut0[BLUE (ip[ 0])] << 10) + lut0[BLUE (ip[ 1])];

  fwd1_r = (lut1[RED  (ip[-5])] >> 10) + lut1[RED  (ip[-4])];
  fwd1_g = (lut1[GREEN(ip[-5])] >> 10) + lut1[GREEN(ip[-4])];
  fwd1_b = (lut1[BLUE (ip[-5])] >> 10) + lut1[BLUE (ip[-4])];

  rev1_r = (lut1[RED  (ip[ 3])] << 10) + lut1[RED  (ip[ 4])];
  rev1_g = (lut1[GREEN(ip[ 3])] << 10) + lut1[GREEN(ip[ 4])];
  rev1_b = (lut1[BLUE (ip[ 3])] << 10) + lut1[BLUE (ip[ 4])];

  while (len--)
  {
    int val_r, val_g, val_b;

    fwd0_r = (fwd0_r >> 10) + lut0[RED  (ip[ 0])];
    fwd0_g = (fwd0_g >> 10) + lut0[GREEN(ip[ 0])];
    fwd0_b = (fwd0_b >> 10) + lut0[BLUE (ip[ 0])];

    rev0_r = (rev0_r << 10) + lut0[RED  (ip[ 2])];
    rev0_g = (rev0_g << 10) + lut0[GREEN(ip[ 2])];
    rev0_b = (rev0_b << 10) + lut0[BLUE (ip[ 2])];

    fwd1_r = (fwd1_r >> 10) + lut1[RED  (ip[-3])];
    fwd1_g = (fwd1_g >> 10) + lut1[GREEN(ip[-3])];
    fwd1_b = (fwd1_b >> 10) + lut1[BLUE (ip[-3])];

    rev1_r = (rev1_r << 10) + lut1[RED  (ip[ 5])];
    rev1_g = (rev1_g << 10) + lut1[GREEN(ip[ 5])];
    rev1_b = (rev1_b << 10) + lut1[BLUE (ip[ 5])];

    val_r = ((fwd0_r & MASK) + ((rev0_r >> 20) & MASK) +
             (fwd1_r & MASK) + ((rev1_r >> 20) & MASK) - bias) >> 2;
    val_r = CLAMP(val_r, 0, 255);

    val_g = ((fwd0_g & MASK) + ((rev0_g >> 20) & MASK) +
             (fwd1_g & MASK) + ((rev1_g >> 20) & MASK) - bias) >> 2;
    val_g = CLAMP(val_g, 0, 255);

    val_b = ((fwd0_b & MASK) + ((rev0_b >> 20) & MASK) +
             (fwd1_b & MASK) + ((rev1_b >> 20) & MASK) - bias) >> 2;
    val_b = CLAMP(val_b, 0, 255);

    *op = FUSE(val_r, val_g, val_b) | (ip[0] & 0xff000000);

    ip++;

    op += stride;
  }
}

static void convolve_888_3stage(const convolve_lut *lut,
                                const Pixel888     *ip,
                                Pixel888           *op,
                                int                 len,
                                int                 bias,
                                int                 stride)
{
  const int      *lut0;
  const int      *lut1;
  const int      *lut2;
  int             fwd0_r, fwd1_r, fwd2_r;
  int             rev0_r, rev1_r, rev2_r;
  int             fwd0_g, fwd1_g, fwd2_g;
  int             rev0_g, rev1_g, rev2_g;
  int             fwd0_b, fwd1_b, fwd2_b;
  int             rev0_b, rev1_b, rev2_b;

  lut0 = lut->lut[0];
  lut1 = lut->lut[1];
  lut2 = lut->lut[2];

  ip += 8; /* ip[0] is centre pixel */

  fwd0_r = (lut0[RED  (ip[-2])] >> 10) + lut0[RED  (ip[-1])];
  fwd0_g = (lut0[GREEN(ip[-2])] >> 10) + lut0[GREEN(ip[-1])];
  fwd0_b = (lut0[BLUE (ip[-2])] >> 10) + lut0[BLUE (ip[-1])];

  rev0_r = (lut0[RED  (ip[ 0])] << 10) + lut0[RED  (ip[ 1])];
  rev0_g = (lut0[GREEN(ip[ 0])] << 10) + lut0[GREEN(ip[ 1])];
  rev0_b = (lut0[BLUE (ip[ 0])] << 10) + lut0[BLUE (ip[ 1])];

  fwd1_r = (lut1[RED  (ip[-5])] >> 10) + lut1[RED  (ip[-4])];
  fwd1_g = (lut1[GREEN(ip[-5])] >> 10) + lut1[GREEN(ip[-4])];
  fwd1_b = (lut1[BLUE (ip[-5])] >> 10) + lut1[BLUE (ip[-4])];

  rev1_r = (lut1[RED  (ip[ 3])] << 10) + lut1[RED  (ip[ 4])];
  rev1_g = (lut1[GREEN(ip[ 3])] << 10) + lut1[GREEN(ip[ 4])];
  rev1_b = (lut1[BLUE (ip[ 3])] << 10) + lut1[BLUE (ip[ 4])];

  fwd2_r = (lut2[RED  (ip[-8])] >> 10) + lut2[RED  (ip[-7])];
  fwd2_g = (lut2[GREEN(ip[-8])] >> 10) + lut2[GREEN(ip[-7])];
  fwd2_b = (lut2[BLUE (ip[-8])] >> 10) + lut2[BLUE (ip[-7])];

  rev2_r = (lut2[RED  (ip[ 6])] << 10) + lut2[RED  (ip[ 7])];
  rev2_g = (lut2[GREEN(ip[ 6])] << 10) + lut2[GREEN(ip[ 7])];
  rev2_b = (lut2[BLUE (ip[ 6])] << 10) + lut2[BLUE (ip[ 7])];

  while (len--)
  {
    int val_r, val_g, val_b;

    fwd0_r = (fwd0_r >> 10) + lut0[RED  (ip[ 0])];
    fwd0_g = (fwd0_g >> 10) + lut0[GREEN(ip[ 0])];
    fwd0_b = (fwd0_b >> 10) + lut0[BLUE (ip[ 0])];

    rev0_r = (rev0_r << 10) + lut0[RED  (ip[ 2])];
    rev0_g = (rev0_g << 10) + lut0[GREEN(ip[ 2])];
    rev0_b = (rev0_b << 10) + lut0[BLUE (ip[ 2])];

    fwd1_r = (fwd1_r >> 10) + lut1[RED  (ip[-3])];
    fwd1_g = (fwd1_g >> 10) + lut1[GREEN(ip[-3])];
    fwd1_b = (fwd1_b >> 10) + lut1[BLUE (ip[-3])];

    rev1_r = (rev1_r << 10) + lut1[RED  (ip[ 5])];
    rev1_g = (rev1_g << 10) + lut1[GREEN(ip[ 5])];
    rev1_b = (rev1_b << 10) + lut1[BLUE (ip[ 5])];

    fwd2_r = (fwd2_r >> 10) + lut2[RED  (ip[-6])];
    fwd2_g = (fwd2_g >> 10) + lut2[GREEN(ip[-6])];
    fwd2_b = (fwd2_b >> 10) + lut2[BLUE (ip[-6])];

    rev2_r = (rev2_r << 10) + lut2[RED  (ip[ 8])];
    rev2_g = (rev2_g << 10) + lut2[GREEN(ip[ 8])];
    rev2_b = (rev2_b << 10) + lut2[BLUE (ip[ 8])];

    val_r = ((fwd0_r & MASK) + ((rev0_r >> 20) & MASK) +
             (fwd1_r & MASK) + ((rev1_r >> 20) & MASK) +
             (fwd2_r & MASK) + ((rev2_r >> 20) & MASK) - bias) >> 2;
    val_r = CLAMP(val_r, 0, 255);

    val_g = ((fwd0_g & MASK) + ((rev0_g >> 20) & MASK) +
             (fwd1_g & MASK) + ((rev1_g >> 20) & MASK) +
             (fwd2_g & MASK) + ((rev2_g >> 20) & MASK) - bias) >> 2;
    val_g = CLAMP(val_g, 0, 255);

    val_b = ((fwd0_b & MASK) + ((rev0_b >> 20) & MASK) +
             (fwd1_b & MASK) + ((rev1_b >> 20) & MASK) +
             (fwd2_b & MASK) + ((rev2_b >> 20) & MASK) - bias) >> 2;
    val_b = CLAMP(val_b, 0, 255);

    *op = FUSE(val_r, val_g, val_b) | (ip[0] & 0xff000000);

    ip++;

    op += stride;
  }
}

static void convolve_888_Nstage(const convolve_lut *lut,
                                const Pixel888     *ip,
                                Pixel888           *op,
                                int                 len,
                                int                 bias,
                                int                 stride)
{
  int             stages;
  int             i;
  const int      *lutN[MAXLUTS];
  int             fwd_r[MAXLUTS];
  int             rev_r[MAXLUTS];
  int             fwd_g[MAXLUTS];
  int             rev_g[MAXLUTS];
  int             fwd_b[MAXLUTS];
  int             rev_b[MAXLUTS];

  stages = lut->stages;

  for (i = 0; i < stages; i++)
    lutN[i] = lut->lut[i];

  ip += 3 * stages - 1; /* ip[0] is centre pixel */

  for (i = 0; i < stages; i++)
  {
    int j,k;

    j = -3 * i - 2;
    k = j + 1;

    fwd_r[i] = (lutN[i][RED  (ip[j])] >> 10) + lutN[i][RED  (ip[k])];
    fwd_g[i] = (lutN[i][GREEN(ip[j])] >> 10) + lutN[i][GREEN(ip[k])];
    fwd_b[i] = (lutN[i][BLUE (ip[j])] >> 10) + lutN[i][BLUE (ip[k])];

    j = 3 * i;
    k = j + 1;

    rev_r[i] = (lutN[i][RED  (ip[j])] << 10) + lutN[i][RED  (ip[k])];
    rev_g[i] = (lutN[i][GREEN(ip[j])] << 10) + lutN[i][GREEN(ip[k])];
    rev_b[i] = (lutN[i][BLUE (ip[j])] << 10) + lutN[i][BLUE (ip[k])];
  }

  while (len--)
  {
    int val_r, val_g, val_b;

    for (i = 0; i < stages; i++)
    {
      int j;

      j = -3 * i;

      fwd_r[i] = (fwd_r[i] >> 10) + lutN[i][RED  (ip[j])];
      fwd_g[i] = (fwd_g[i] >> 10) + lutN[i][GREEN(ip[j])];
      fwd_b[i] = (fwd_b[i] >> 10) + lutN[i][BLUE (ip[j])];

      j = 3 * i + 2;

      rev_r[i] = (rev_r[i] << 10) + lutN[i][RED  (ip[j])];
      rev_g[i] = (rev_g[i] << 10) + lutN[i][GREEN(ip[j])];
      rev_b[i] = (rev_b[i] << 10) + lutN[i][BLUE (ip[j])];
    }

    val_r = 0;
    val_g = 0;
    val_b = 0;

    for (i = 0; i < stages; i++)
    {
      val_r += (fwd_r[i] & MASK) + ((rev_r[i] >> 20) & MASK);
      val_g += (fwd_g[i] & MASK) + ((rev_g[i] >> 20) & MASK);
      val_b += (fwd_b[i] & MASK) + ((rev_b[i] >> 20) & MASK);
    }

    val_r = (val_r - bias) >> 2;
    val_g = (val_g - bias) >> 2;
    val_b = (val_b - bias) >> 2;

    val_r = CLAMP(val_r, 0, 255);
    val_g = CLAMP(val_g, 0, 255);
    val_b = CLAMP(val_b, 0, 255);

    *op = FUSE(val_r, val_g, val_b) | (ip[0] & 0xff000000);

    ip++;

    op += stride;
  }
}

static error convolve_888(const convolve_lut *lut,
                          const Pixel888     *src,
                          Pixel888           *dst,
                          int                 len,
                          int                 stride,
                          Pixel888           *buf)
{
  int              padlen;
  const Pixel888  *p1;
  Pixel888        *p2;
  int              x;
  const Pixel888  *ip;
  Pixel888        *op;
  convolve_888_fn *fn;

  int              bias;

  /* buffer and pad the input */

  padlen = 3 * lut->stages - 1;
  p1 = src;
  p2 = buf;
  for (x = 0; x < padlen; x++)
    *p2++ = *p1;
  for (x = 0; x < len; x++)
  {
    *p2++ = *p1;
    p1 += stride;
  }
  p1 -= stride;
  for (x = 0; x < padlen; x++)
    *p2++ = *p1;

  ip = buf;
  op = dst;

  bias = lut->bias;

  switch (lut->stages)
  {
  case 1: /* 5-pt kernel */
    fn = convolve_888_1stage;
    break;
  case 2: /* 11-pt kernel */
    fn = convolve_888_2stage;
    break;
  case 3: /* 17-pt kernel */
    fn = convolve_888_3stage;
    break;
  case 4: /* 23-pt kernel */
  case 5: /* 29-pt kernel */
  case 6: /* 35-pt kernel */
  case 7: /* 41-pt kernel */
  case 8: /* 47-pt kernel */
    fn = convolve_888_Nstage;
    break;

  default:
    return error_SPRITEFX_UNSUPP_EFFECT;
  }

  fn(lut, ip, op, len, lut->bias, stride);

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error convolve_sprite(const convolve_lut      *lut,
                      const osspriteop_area   *area,
                      const osspriteop_header *src,
                      osspriteop_header       *dst)
{
  error                 err;
  int                   width, height;
  osspriteop_mode_word  mode;
  int                   log2bpp;
  int                   maxdim;
  int                   x,y;

  sprite_info(area, src, &width, &height, NULL, (os_mode *) &mode, &log2bpp);

  /* Allow for the maximum dimension of the sprite,
   * plus padding at either end. */
  maxdim = MAX(width, height) + (3 * lut->stages - 1) * 2;

  switch (log2bpp)
  {
  case 3:
  {
    Pixel8               *row;
    int                   rowbytes;
    const Pixel8         *sp;
    Pixel8               *dp;

    row = malloc(maxdim * sizeof(*row));
    if (row == NULL)
    {
      err = error_OOM;
      goto Failure;
    }

    rowbytes = (width + 3) & ~3;

    sp = (const Pixel8 *) sprite_data(src);
    dp = (Pixel8 *) sprite_data(dst);
    for (y = 0; y < height; y++)
    {
      (void) convolve_8(lut, sp, dp, width, 1, row); /* src -> dst */
      sp += rowbytes;
      dp += rowbytes;
    }

    sp = (const Pixel8 *) sprite_data(dst); /* dst -> dst */
    dp = (Pixel8 *) sprite_data(dst);
    for (x = 0; x < width; x++)
    {
      (void) convolve_8(lut, sp, dp, height, width, row);
      sp++;
      dp++;
    }

    free(row);
  }
    break;

  case 5:
  {
    Pixel888             *row;
    const Pixel888       *sp;
    Pixel888             *dp;

    row = malloc(maxdim * sizeof(*row));
    if (row == NULL)
    {
      err = error_OOM;
      goto Failure;
    }

    sp = (const Pixel888 *) sprite_data(src); /* src -> dst */
    dp = (Pixel888 *) sprite_data(dst);
    for (y = 0; y < height; y++)
    {
      (void) convolve_888(lut, sp, dp, width, 1, row);
      sp += width;
      dp += width;
    }

    sp = (const Pixel888 *) sprite_data(dst); /* dst -> dst */
    dp = (Pixel888 *) sprite_data(dst);
    for (x = 0; x < width; x++)
    {
      (void) convolve_888(lut, sp, dp, height, width, row);
      sp++;
      dp++;
    }

    free(row);
  }
    break;

  default:
    err = error_SPRITEFX_UNSUPP_EFFECT;
    goto Failure;
  }

  return error_OK;


Failure:

  return err;
}
