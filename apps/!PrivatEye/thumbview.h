/* --------------------------------------------------------------------------
 *    Name: thumbview.h
 * Purpose: Filer-like thumbnail viewing
 * ----------------------------------------------------------------------- */

#ifndef THUMBVIEW_H
#define THUMBVIEW_H

#include "appengine/base/errors.h"

typedef struct thumbview thumbview;

error thumbview_substrate_init(void);

error thumbview_init(void);
void thumbview_fin(void);

error thumbview_create(thumbview **new_tv);
void thumbview_destroy(thumbview *doomed);

void thumbview_load_dir(thumbview *tv, const char *dir_name);

void thumbview_open(thumbview *tv);

void thumbview_close_all(void);

int thumbview_get_count(void);

#endif /* THUMBVIEW_H */
