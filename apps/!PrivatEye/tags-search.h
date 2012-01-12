/* --------------------------------------------------------------------------
 *    Name: tags-search.h
 * Purpose: Searching for tagged images
 * ----------------------------------------------------------------------- */

#ifndef TAGS_SEARCH_H
#define TAGS_SEARCH_H

#include "appengine/base/errors.h"

error tags_search_init(void);
void tags_search_fin(void);
error tags_search_open(void);

#endif /* TAGS_SEARCH_H */
