/* --------------------------------------------------------------------------
 *    Name: card.h
 * Purpose: Draws regions bordered by sprites
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_CARD_H
#define APPENGINE_CARD_H

#include "oslib/wimp.h"

void card__prepare(int item_w, int item_h);

typedef unsigned int card__draw_flags;
#define card__draw_flag_INVERT (1u << 0)

void card__draw(wimp_draw *redraw, int x, int y, card__draw_flags flags);

#endif /* APPENGINE_CARD_H */
