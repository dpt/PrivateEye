/* --------------------------------------------------------------------------
 *    Name: choices-colour.c
 * Purpose: Colour correction choices
 * ----------------------------------------------------------------------- */

#include "fortify/fortify.h"

#include "appengine/app/choices.h"
#include "appengine/base/messages.h"
#include "appengine/graphics/tonemap.h"
#include "appengine/gadgets/tonemap-gadget.h"
#include "appengine/vdu/screen.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/colourtrans.h"
#include "oslib/wimp.h"

#include "privateeye.h"
#include "iconnames.h"
#include "menunames.h"
#include "globals.h"

#include "choices-colour.h"

enum
{
  Border = 8  /* size of an icon border */
};

/* ----------------------------------------------------------------------- */

static struct
{
  tonemap *map;
  int      on;
}
LOCALS;

/* ----------------------------------------------------------------------- */

error colour_choicepane_initialise(const choices_pane *p)
{
  NOT_USED(p);

  LOCALS.map = tonemap_create();
  if (LOCALS.map == NULL)
    return error_OOM;

  return error_OK;
}

void colour_choicepane_finalise(const choices_pane *p)
{
  NOT_USED(p);

  tonemap_destroy(LOCALS.map);
  LOCALS.map = NULL;
}

error colour_choicepane_changed(const choices_pane *p)
{
  tonemap      *temp_map;
  tonemap      *screen_map;
  tonemap_spec  spec;

  spec.flags      = 0;
  spec.gamma      = GLOBALS.proposed_choices.colour.gamma;
  spec.brightness = GLOBALS.proposed_choices.colour.brightness;
  spec.contrast   = GLOBALS.proposed_choices.colour.contrast;
  spec.midd       = 50;
  spec.bias       = 50;
  spec.gain       = 50;

  tonemap_set(LOCALS.map, tonemap_CHANNEL_RGB, &spec);

  tonemapgadget_update(LOCALS.map, *p->window, CHOICES_COL_D_CURVE);

  temp_map = NULL;

  if (GLOBALS.proposed_choices.colour.on)
  {
    screen_map = LOCALS.map;
  }
  else
  {
    if (LOCALS.on)
    {
      static const tonemap_spec spec = { 0, 100, 100, 100, 50, 50, 50 };

      /* currently off + previously turned on */

      /* reset screen to default */

      temp_map = tonemap_create();
      if (temp_map == NULL)
        return error_OOM;

      tonemap_set(temp_map, tonemap_CHANNEL_RGB, &spec);

      screen_map = temp_map;
    }
    else
    {
      /* currently off + previously turned off */

      screen_map = NULL; /* do nothing */
    }
  }

  if (screen_map)
  {
    const os_correction_table *red, *green, *blue;

    tonemap_get_corrections(screen_map, &red, &green, &blue, NULL);
    palettev_set_gamma_corrections(red, green, blue);
  }

  tonemap_destroy(temp_map);

  LOCALS.on = GLOBALS.proposed_choices.colour.on;

  return error_OK;
}

error colour_choicepane_redraw(const choices_pane *p, wimp_draw *redraw)
{
  tonemapgadget_redraw(LOCALS.map, *p->window, CHOICES_COL_D_CURVE, redraw);

  return error_OK;
}
