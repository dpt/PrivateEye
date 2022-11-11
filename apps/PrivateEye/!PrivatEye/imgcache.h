/* --------------------------------------------------------------------------
 *    Name: imgcache.h
 * Purpose: Image cache
 * ----------------------------------------------------------------------- */

#ifndef IMGCACHE_H
#define IMGCACHE_H

#include "appengine/base/errors.h"
#include "appengine/app/choices.h"

result_t imgcache_init(void);
void imgcache_fin(void);
result_t imgcache_choices_updated(const choices_group *group);

#endif /* IMGCACHE_H */
