/* --------------------------------------------------------------------------
 *    Name: tags-search.h
 * Purpose: Searching for tagged images
 * Version: $Id: tags-search.h,v 1.4 2009-05-20 21:38:19 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef SEARCHTAGS_H
#define SEARCHTAGS_H

#include "appengine/base/errors.h"

error tags_search__init(void);
void tags_search__fin(void);
error tags_search__open(void);
//int tags_search__available(void);

#endif /* SEARCHTAGS_H */
