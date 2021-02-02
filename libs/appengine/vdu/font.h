/* --------------------------------------------------------------------------
 *    Name: font.h
 * Purpose: Declarations for the font library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_FONT_H
#define APPENGINE_FONT_H

#include "appengine/base/errors.h"

typedef unsigned int font_attrs;

#define font_WEIGHT_MASK       (0xf << 0)

#define font_WEIGHT_100        (0 << 0)
#define font_WEIGHT_200        (1 << 0)
#define font_WEIGHT_300        (2 << 0)
#define font_WEIGHT_400        (3 << 0)
#define font_WEIGHT_500        (4 << 0)
#define font_WEIGHT_600        (5 << 0)
#define font_WEIGHT_700        (6 << 0)
#define font_WEIGHT_800        (7 << 0)
#define font_WEIGHT_900        (8 << 0)
#define font_WEIGHT_950        (9 << 0) /* ideally this should be 9.5 */

#define font_WEIGHT_LIGHT      font_WEIGHT_300
#define font_WEIGHT_NORMAL     font_WEIGHT_400
#define font_WEIGHT_MEDIUM     font_WEIGHT_500
#define font_WEIGHT_BOLD       font_WEIGHT_700
#define font_WEIGHT_BLACK      font_WEIGHT_900

#define font_STYLE_MASK        (0xf << 4)

#define font_STYLE_NORMAL      (0 << 4)
#define font_STYLE_ITALIC      (1 << 4)
#define font_STYLE_OBLIQUE     (2 << 4)
#define font_STYLE_SLANTED     (3 << 4)

#define font_STRETCH_MASK      (0xf << 8)

#define font_STRETCH_ULTRACOND (0 << 8)
#define font_STRETCH_EXTRACOND (1 << 8)
#define font_STRETCH_COND      (2 << 8)
#define font_STRETCH_SEMICOND  (3 << 8)
#define font_STRETCH_NORMAL    (4 << 8)
#define font_STRETCH_SEMIEXT   (5 << 8)
#define font_STRETCH_EXT       (6 << 8)
#define font_STRETCH_EXTRAEXT  (7 << 8)
#define font_STRETCH_ULTRAEXT  (8 << 8)

#define font_VARIANT_MASK      (0xf << 12)

#define font_VARIANT_NORMAL    (0 << 12)
#define font_VARIANT_SMALLCAPS (1 << 12)
#define font_VARIANT_TITLE     (2 << 12)
#define font_VARIANT_OPEN      (3 << 12)
#define font_VARIANT_OSF       (4 << 12) /* Old Style Figures */
#define font_VARIANT_EXPERT    (5 << 12)
#define font_VARIANT_POSTER    (6 << 12)

/* Given a font name and attrs returns the name of a suitable face in
 * 'selected'.
 *
 * Returns error_FONT_NO_MATCH where no match is found.
 */

result_t font_select(const char *name,
                     font_attrs  attrs,
                     char       *selected,
                     int         selected_size);
font_attrs font_get_attrs(const char *name);

#endif /* APPENGINE_FONT_H */
