/* --------------------------------------------------------------------------
 *    Name: tags-search.h
 * Purpose: Searching for tagged images
 * Version: $Id: tags-search.h,v 1.4 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef TAGS_SEARCH_H
#define TAGS_SEARCH_H

#include "appengine/base/errors.h"

error tags_search__init(void);
void tags_search__fin(void);
error tags_search__open(void);

#endif /* TAGS_SEARCH_H */
