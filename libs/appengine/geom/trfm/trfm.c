/* --------------------------------------------------------------------------
 *    Name: trfm.c
 * Purpose: Routines for dealing with transforms
 * ----------------------------------------------------------------------- */

#include <math.h>
#include <stdio.h>
#include <float.h>

#include "fortify/fortify.h"

#include "oslib/os.h"

#include "appengine/geom/trfm.h"
#include "appengine/base/utils.h"

/* This should really be 16, but that makes many of the multiplies overflow.
 * So, for now, I'm leaving it at 8 and will scale up later. */
#define SCALE 8

static const os_trfm identity = { { { 1 << SCALE, 0 << SCALE },
                                    { 0 << SCALE, 1 << SCALE },
                                    { 0 << SCALE, 0 << SCALE } } };

void trfm_set_identity(os_trfm *t)
{
  *t = identity;
}

static void update_general(os_trfm *u, const os_trfm *t)
{
  int     p, q;
  os_trfm v;

  p = u->entries[0][0] * t->entries[0][0];
  q = u->entries[0][1] * t->entries[1][0];
  v.entries[0][0] = (p + q) >> SCALE;

  q = u->entries[0][0] * t->entries[0][1];
  p = u->entries[0][1] * t->entries[1][1];
  v.entries[0][1] = (p + q) >> SCALE;

  p = u->entries[1][0] * t->entries[0][0];
  q = u->entries[1][1] * t->entries[1][0];
  v.entries[1][0] = (p + q) >> SCALE;

  q = u->entries[1][0] * t->entries[0][1];
  p = u->entries[1][1] * t->entries[1][1];
  v.entries[1][1] = (p + q) >> SCALE;

  p = u->entries[2][0] * t->entries[0][0];
  q = u->entries[2][1] * t->entries[1][0];
  v.entries[2][0] = ((p + q) >> SCALE) + t->entries[2][0];

  q = u->entries[2][0] * t->entries[0][1];
  p = u->entries[2][1] * t->entries[1][1];
  v.entries[2][1] = ((p + q) >> SCALE) + t->entries[2][1];

  *u = v;
}

static void update_scale(os_trfm *u, const os_trfm *t)
{
  int     p;
  os_trfm v;

  p = u->entries[0][0] * t->entries[0][0];
  v.entries[0][0] = (p >> SCALE);

  p = u->entries[0][1] * t->entries[1][1];
  v.entries[0][1] = (p >> SCALE);

  p = u->entries[1][0] * t->entries[0][0];
  v.entries[1][0] = (p >> SCALE);

  p = u->entries[1][1] * t->entries[1][1];
  v.entries[1][1] = (p >> SCALE);

  p = u->entries[2][0] * t->entries[0][0];
  v.entries[2][0] = (p >> SCALE) + t->entries[2][0];

  p = u->entries[2][1] * t->entries[1][1];
  v.entries[2][1] = (p >> SCALE) + t->entries[2][1];

  *u = v;
}

static void update_translate(os_trfm *u, const os_trfm *t)
{
  u->entries[2][0] += t->entries[2][0];
  u->entries[2][1] += t->entries[2][1];
}

void trfm_update(os_trfm *update, const os_trfm *transform)
{
  if (transform->entries[0][1] != 0 || transform->entries[1][0] != 0)
  {
    update_general(update, transform);
  }
  else
  {
    if (transform->entries[0][0] != 1 << SCALE || transform->entries[1][1] != 1 << SCALE)
    {
      update_scale(update, transform);
    }
    else
    {
      update_translate(update, transform);
    }
  }
}

void trfm_rotate_degs(os_trfm *transform, int angle)
{
  os_trfm t;
  double  radians;

  if (0 == angle)
    return;

  radians = degs_to_rads(angle / 65536.0);

  t.entries[0][0] = (int) (cos(radians) * (1 << SCALE));
  t.entries[0][1] = (int) (sin(radians) * (1 << SCALE));
  t.entries[1][0] = -t.entries[0][1];
  t.entries[1][1] =  t.entries[0][0];
  t.entries[2][0] = 0;
  t.entries[2][1] = 0;

  trfm_update(transform, &t);
}

void trfm_rotate_rads(os_trfm *transform, int angle)
{
  os_trfm t;
  double  radians;

  if (0 == angle)
    return;

  radians = angle / 65536.0;

  t.entries[0][0] = (int) (cos(radians) * (1 << SCALE));
  t.entries[0][1] = (int) (sin(radians) * (1 << SCALE));
  t.entries[1][0] = -t.entries[0][1];
  t.entries[1][1] =  t.entries[0][0];
  t.entries[2][0] = 0;
  t.entries[2][1] = 0;

  trfm_update(transform, &t);
}

void trfm_translate(os_trfm *transform, int x, int y)
{
  os_trfm t;

  t.entries[0][0] = 1 << SCALE;
  t.entries[0][1] = 0;
  t.entries[1][0] = 0;
  t.entries[1][1] = 1 << SCALE;
  t.entries[2][0] = x;
  t.entries[2][1] = y;

  trfm_update(transform, &t);
}

#if 0

#include <stdlib.h>

#include "oslib/osspriteop.h"

#define SIZE 128*1024
#define CX 640
#define CY 480

double getangle(int x, int y)
{
/*
  // this seems to lose it near the vertical origin

  double l;

  l = sqrt((double) x * x + (double) y * y);
  if (l > DBL_EPSILON)
  {
    double angle;

    angle = ((double) y / l) * 90.0;

    if (x < 0)
      return 90.0 - angle;
    else
      return angle - 90.0;
  }
*/

  if (x != 0 || y != 0)
    return (atan2(y, x) / (2 * 3.14159265)) * 360.0 - 90.0;

  return 0.0;
}

int main(void)
{
  osspriteop_area *area;
  os_trfm          t;
  int              w,h;
  osbool           mask;
  os_mode          mode;
  int              oldx,oldy;
  bits             oldbuttons;
  os_t             time;

  int  dragging = 0;
  int  clickx, clicky;
  double totalangle = 0;
  double startangle = 0;
  double lastangle  = 0;


  area = malloc(SIZE);

  w = CX;
  h = CY;
  os_set_graphics_origin();
  os_writec(w & 0xff);
  os_writec(w >> 8);
  os_writec(h & 0xff);
  os_writec(h >> 8);

  area->size = SIZE;
  area->sprite_count = 0;
  area->first = 16;
  area->used = 16;
  osspriteop_clear_sprites(osspriteop_USER_AREA, area);

  osspriteop_load_sprite_file(osspriteop_USER_AREA, area, "enlarged-sprmask");

  osspriteop_read_sprite_info(osspriteop_USER_AREA,
                              area,
             (osspriteop_id) "example",
                              &w, &h, &mask, &mode);

  os_mouse(&oldx, &oldy, &oldbuttons, &time);

  for (;;)
  {
    int  x,y;
    bits buttons;

    os_mouse(&x, &y, &buttons, &time);

    if (x != oldx || y != oldy || buttons != oldbuttons)
    {
      if ((buttons & 4) != 0 && (oldbuttons & 4) == 0)
      {
        dragging = 1;

        clickx = x - 0;
        clicky = y - 0;

        startangle = getangle(clickx, clicky);
      }

      if ((buttons & 4) == 0 && (oldbuttons & 4) != 0)
      {
        dragging = 0;

        totalangle += lastangle;
        printf("%f\n", totalangle);
      }

      oldx = x;
      oldy = y;
      oldbuttons = buttons;

      if (dragging)
      {
        lastangle = getangle(x - 0, y - 0) - startangle;

        printf("change=%f\n", lastangle);

        trfm_set_identity(&t);
        trfm_translate(&t, -w / 2 * 2, -h / 2 * 2);
        trfm_rotate(&t, (int) ((totalangle + lastangle) * 65536.0));

        t.entries[0][0] *= 256;
        t.entries[0][1] *= 256;
        t.entries[1][0] *= 256;
        t.entries[1][1] *= 256;
        t.entries[2][0] *= 256;
        t.entries[2][1] *= 256;

        osspriteop_put_sprite_trfm(osspriteop_NAME,
                                   area,
                  (osspriteop_id) "example",
                                   0, /* trfm_flags */
                                   NULL, /* source_rect */
                                   osspriteop_USE_MASK,
                                   &t,
                                   NULL);
      }
    }
  }
}

#endif
