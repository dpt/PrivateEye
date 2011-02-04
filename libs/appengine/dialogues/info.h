/* --------------------------------------------------------------------------
 *    Name: info.h
 * Purpose: Info dialogue
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_DIALOGUE_INFO_H
#define APPENGINE_DIALOGUE_INFO_H

#include "oslib/types.h"
#include "oslib/wimp.h"

#include "appengine/wimp/dialogue.h"
#include "appengine/base/errors.h"

#define T info_t

/* ----------------------------------------------------------------------- */

typedef struct T
{
  dialogue_t dialogue; /* base class */

  wimp_i     file_type_icon;
  wimp_i     displays[8];
  int        ndisplays;

  int        padding; /* in OS units */
}
T;

/* ----------------------------------------------------------------------- */

dialogue_t *info__create(const char *template);
void info__destroy(dialogue_t *d);

void info__construct(T *s, const char *template);
void info__destruct(T *s);

typedef struct info_spec_t
{
  const char *value;
}
info_spec_t;

/* Sets the dialogue to display a file type e.g. when used as a prog info
 * window. */
void info__set_file_type(dialogue_t *d, bits file_type);

/* Sets the values of all the display icons in the window.
 * This is done in icon order. */
void info__set_info(dialogue_t *d, info_spec_t *specs, int nspecs);

/* Call this once info and icon type have been set to layout the dialogue,
 * or don't bother if it's a fixed layout. */
void info__layout(dialogue_t *d);

/* Sets the amount of padding to apply to fields, in OS units.
 * The padding is split across both ends of the field. */
void info__set_padding(dialogue_t *d, int padding);

/* ----------------------------------------------------------------------- */

#undef T

#endif /* APPENGINE_DIALOGUE_INFO_H */
