/* --------------------------------------------------------------------------
 *    Name: scale.c
 * Purpose: Scale dialogue
 * ----------------------------------------------------------------------- */

#include <assert.h>

#include "fortify/fortify.h"

#include "oslib/osbyte.h"
#include "oslib/wimp.h"

#include "appengine/types.h"
#include "appengine/wimp/dialogue.h"
#include "appengine/wimp/event.h"
#include "appengine/wimp/icon.h"
#include "appengine/base/os.h"

#include "appengine/dialogues/scale.h"

/* ----------------------------------------------------------------------- */

/* Icons for window "scale" */
enum
{
  SCALE_B_25PC   = 0,
  SCALE_B_50PC   = 1,
  SCALE_B_100PC  = 2,
  SCALE_B_200PC  = 3,
  SCALE_B_FITSCR = 4,
  SCALE_B_FITWIN = 5,
  SCALE_B_CANCEL = 6,
  SCALE_B_SCALE  = 7,
  SCALE_W_SCALE  = 9,
  SCALE_B_DOWN   = 10,
  SCALE_B_UP     = 11,
};

/* ----------------------------------------------------------------------- */

typedef struct scale_t
{
  dialogue_t            dialogue; /* base class */
  int                   min, max;
  int                   step, mult;
  scale_scale_handler *scale_handler;
}
scale_t;

/* ----------------------------------------------------------------------- */

static event_wimp_handler scale_event_mouse_click,
                          scale_event_key_pressed;

/* ----------------------------------------------------------------------- */

dialogue_t *scale_create(void)
{
  scale_t *s;

  s = calloc(1, sizeof(*s));
  if (s == NULL)
    return NULL;

  dialogue_construct(&s->dialogue, "scale", SCALE_B_SCALE, SCALE_B_CANCEL);

  /* set defaults */
  s->min  = 1;
  s->max  = 8000;
  s->step = 1;
  s->mult = 1;

  dialogue_set_handlers(&s->dialogue,
                         scale_event_mouse_click,
                         scale_event_key_pressed,
                         NULL);

  return &s->dialogue;
}

void scale_destroy(dialogue_t *d)
{
  scale_t *s;

  s = (scale_t *) d;

  dialogue_destruct(&s->dialogue);

  free(s);
}

/* ----------------------------------------------------------------------- */

static void scale_nudge(scale_t *s, int up, int faster)
{
  wimp_w win;
  int    scale;
  int    inc;

  win = dialogue_get_window(&s->dialogue);

  scale = icon_get_int(win, SCALE_W_SCALE);

  inc = s->step;

  if (faster)
    inc *= s->mult;

  if (!up)
    inc = -inc;

  scale += inc;

  /* round to nearest multiple */
  scale += abs(inc) / 2;
  scale /= inc;
  scale *= inc;

  scale_set(&s->dialogue, scale);
}

static int scale_event_mouse_click(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_pointer *pointer;
  scale_t      *s;

  NOT_USED(event_no);

  pointer = &block->pointer;
  s       = handle;

  if (pointer->buttons & (wimp_CLICK_SELECT | wimp_CLICK_ADJUST))
  {
    switch (pointer->i)
    {
    case SCALE_B_SCALE:
      {
        wimp_w win;
        int    scale;

        win = dialogue_get_window(&s->dialogue);

        scale = icon_get_int(win, SCALE_W_SCALE);

        scale_set(&s->dialogue, scale);

        s->scale_handler(&s->dialogue, scale_TYPE_VALUE, scale);
      }
      break;

    case SCALE_B_UP:
    case SCALE_B_DOWN:
      {
        int up;
        int faster;

        up = (pointer->i == SCALE_B_UP   && pointer->buttons & 4) ||
             (pointer->i == SCALE_B_DOWN && pointer->buttons & 1);

        faster = inkey(INKEY_SHIFT);

        scale_nudge(s, up, faster);
      }
      break;

    case SCALE_B_25PC:
    case SCALE_B_50PC:
    case SCALE_B_100PC:
    case SCALE_B_200PC:
      {
        int scale;

        scale = (pointer->i == SCALE_B_25PC)  ?  25 :
                (pointer->i == SCALE_B_50PC)  ?  50 :
                (pointer->i == SCALE_B_100PC) ? 100 :
                                                200;

        scale_set(&s->dialogue, scale);
      }
      break;

    case SCALE_B_FITSCR:
      /* handler calls scale_set */
      s->scale_handler(&s->dialogue, scale_TYPE_FIT_TO_SCREEN, 0);
      break;

    case SCALE_B_FITWIN:
      /* handler calls scale_set */
      s->scale_handler(&s->dialogue, scale_TYPE_FIT_TO_WINDOW, 0);
      break;
    }
  }

  return event_HANDLED;
}

static int scale_event_key_pressed(wimp_event_no event_no, wimp_block *block, void *handle)
{
  wimp_key *key;
  scale_t  *s;

  NOT_USED(event_no);

  key = &block->key;
  s   = handle;

  switch (key->c)
  {
  case wimp_KEY_UP:
  case wimp_KEY_DOWN:
  case wimp_KEY_SHIFT | wimp_KEY_UP:
  case wimp_KEY_SHIFT | wimp_KEY_DOWN:
    {
      int up;
      int faster;

      up = (key->c & ~wimp_KEY_SHIFT) == wimp_KEY_UP;

      faster = key->c & wimp_KEY_SHIFT;

      scale_nudge(s, up, faster);

      /* cause an immediate update */
      s->scale_handler(&s->dialogue, scale_TYPE_VALUE,
                       scale_get(&s->dialogue));
    }
    break;

  default:
    wimp_process_key(key->c);
    return event_HANDLED;
  }

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

void scale_set(dialogue_t *d, int scale)
{
  scale_t *s = (scale_t *) d;
  wimp_w   win;

  scale = CLAMP(scale, s->min, s->max);

  win = dialogue_get_window(d);

  icon_set_int(win, SCALE_W_SCALE, scale);
}

int scale_get(dialogue_t *d)
{
  scale_t *s = (scale_t *) d;
  wimp_w   win;
  int      scale;

  win = dialogue_get_window(d);

  scale = icon_get_int(win, SCALE_W_SCALE);

  /* User may have modified it, so clamp. */

  scale = CLAMP(scale, s->min, s->max);

  return scale;
}

void scale_set_range(dialogue_t *d, int min, int max)
{
  scale_t *s = (scale_t *) d;

  assert(max > min);

  s->min = min;
  s->max = max;
}

void scale_set_steppings(dialogue_t *d, int step, int mult)
{
  scale_t *s = (scale_t *) d;

  assert(step > 0);
  assert(mult > 0);

  s->step = step;
  s->mult = mult;
}

void scale_set_scale_handler(dialogue_t *d, scale_scale_handler *scale_handler)
{
  scale_t *s = (scale_t *) d;

  s->scale_handler = scale_handler;
}
