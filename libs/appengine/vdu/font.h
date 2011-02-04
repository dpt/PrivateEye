/* --------------------------------------------------------------------------
 *    Name: font.h
 * Purpose: Declarations for the font library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_FONT_H
#define APPENGINE_FONT_H

#include "appengine/base/errors.h"

typedef unsigned int font__attrs;

#define font__WEIGHT_MASK       (0xf << 0)

#define font__WEIGHT_100        (0 << 0)
#define font__WEIGHT_200        (1 << 0)
#define font__WEIGHT_300        (2 << 0)
#define font__WEIGHT_400        (3 << 0)
#define font__WEIGHT_500        (4 << 0)
#define font__WEIGHT_600        (5 << 0)
#define font__WEIGHT_700        (6 << 0)
#define font__WEIGHT_800        (7 << 0)
#define font__WEIGHT_900        (8 << 0)
#define font__WEIGHT_950        (9 << 0) /* ideally this should be 9.5 */

#define font__WEIGHT_LIGHT      font__WEIGHT_300
#define font__WEIGHT_NORMAL     font__WEIGHT_400
#define font__WEIGHT_MEDIUM     font__WEIGHT_500
#define font__WEIGHT_BOLD       font__WEIGHT_700
#define font__WEIGHT_BLACK      font__WEIGHT_900

#define font__STYLE_MASK        (0xf << 4)

#define font__STYLE_NORMAL      (0 << 4)
#define font__STYLE_ITALIC      (1 << 4)
#define font__STYLE_OBLIQUE     (2 << 4)
#define font__STYLE_SLANTED     (3 << 4)

#define font__STRETCH_MASK      (0xf << 8)

#define font__STRETCH_ULTRACOND (0 << 8)
#define font__STRETCH_EXTRACOND (1 << 8)
#define font__STRETCH_COND      (2 << 8)
#define font__STRETCH_SEMICOND  (3 << 8)
#define font__STRETCH_NORMAL    (4 << 8)
#define font__STRETCH_SEMIEXT   (5 << 8)
#define font__STRETCH_EXT       (6 << 8)
#define font__STRETCH_EXTRAEXT  (7 << 8)
#define font__STRETCH_ULTRAEXT  (8 << 8)

#define font__VARIANT_MASK      (0xf << 12)

#define font__VARIANT_NORMAL    (0 << 12)
#define font__VARIANT_SMALLCAPS (1 << 12)
#define font__VARIANT_TITLE     (2 << 12)
#define font__VARIANT_OPEN      (3 << 12)
#define font__VARIANT_OSF       (4 << 12) /* Old Style Figures */
#define font__VARIANT_EXPERT    (5 << 12)
#define font__VARIANT_POSTER    (6 << 12)

/* Given a font name and attrs returns the name of a suitable face in
 * 'selected'.
 *
 * Returns error_FONT_NO_MATCH where no match is found.
 */
error font__select(const char  *name,
                   font__attrs  attrs,
                   char        *selected,
                   int          selected_size);

font__attrs font__get_attrs(const char *name);

#endif /* APPENGINE_FONT_H */
