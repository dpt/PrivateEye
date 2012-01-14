
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/fileswitch.h"
#include "oslib/osfile.h"
#include "oslib/osspriteop.h"
#include "oslib/wimpreadsysinfo.h"

#include "appengine/base/strings.h"
#include "appengine/base/errors.h"

#include "appengine/wimp/window.h"

static osspriteop_area *template_sprite_area = NULL;

static int _load_sprites(const char *spritefile, int area_size)
{
  /* Claim memory for area, initialise area, then load sprite file */
  area_size += 4;

  template_sprite_area = malloc(area_size);
  if (template_sprite_area == NULL)
    error_fatal_oom();

  template_sprite_area->size  = area_size;
  template_sprite_area->first = 16;

  osspriteop_clear_sprites(osspriteop_USER_AREA, template_sprite_area);

  osspriteop_load_sprite_file(osspriteop_USER_AREA,
                              template_sprite_area,
                              spritefile);

  return 0;
}

void window_load_sprites(const char *filename)
{
  const char *suffix;
  char       *suffixed;
  int         obj_type;
  int         area_size;

  /* Get the 'Sprites' file suffix pointer */
  suffix = wimpreadsysinfo_sprite_suffix();

  suffixed = malloc(strlen(filename) + strlen(suffix) + 1);
  if (suffixed == NULL)
    error_fatal_oom();

  strcpy(suffixed, filename);
  strcat(suffixed, suffix);

  /* Read suffixed object type and length */
  obj_type = osfile_read_no_path(suffixed,
                                 NULL,
                                 NULL,
                                &area_size,
                                 NULL);
  if (obj_type == fileswitch_IS_FILE)
  {
    _load_sprites(suffixed, area_size);
  }
  else
  {
    /* Suffixed spritefile not found, try the original filename */
    obj_type = osfile_read_no_path(filename,
                                   NULL,
                                   NULL,
                                  &area_size,
                                   NULL);
    if (obj_type != fileswitch_IS_FILE)
      error_fatal1("NoFile", filename);

    _load_sprites(filename, area_size);
  }

  free(suffixed);

  /* MemCheck_UnRegisterMiscBlock(suffix); */
}

void window_set_sprite_area(osspriteop_area *new_sprite_area)
{
  template_sprite_area = new_sprite_area;
}

osspriteop_area *window_get_sprite_area(void)
{
  return template_sprite_area;
}
