
#include <stddef.h>

#include "fortify/fortify.h"

#include "appengine/base/errors.h"
#include "appengine/base/strings.h"

#include "appengine/wimp/window.h"

#define IT (wimp_ICON_TEXT | wimp_ICON_INDIRECTED)

/* FIXME: rewrite this so that we scan for required memory size before we
 *        start duplicating strings, so avoid lots of strdup allocs */

/* FIXME: this won't duplicate indirected sprite icons */

static void clone_icon(wimp_icon_data *data)
{
  char *p;

  p = data->indirected_text.text;
  if (p != NULL && p != (char *) -1)
  {
    p = str_n_dup(p, data->indirected_text.size);
    if (p == NULL)
      goto oom;

    data->indirected_text.text = p;
  }

  p = data->indirected_text.validation;
  if (p != NULL && p != (char *) -1)
  {
    p = str_dup(p);
    if (p == NULL)
      goto oom;

    data->indirected_text.validation = p;
  }

  return;


oom:

  error__fatal_oom();
}

wimp_w window_clone(wimp_w w)
{
  wimp_window_info  info;
  wimp_window_info *defn;
  int               i;
  wimp_w            new_w;

  /* discover the size of array needed to clone the window */

  info.w = w;
  wimp_get_window_info_header_only(&info);

  /* this is one icon larger than it strictly needs to be */
  defn = malloc(sizeof(*defn) + info.icon_count * sizeof(wimp_icon));
  if (defn == NULL)
    error__fatal_oom();

  /* clone the window */

  defn->w = w;
  wimp_get_window_info(defn);

  /* title */

  if ((defn->flags & wimp_WINDOW_TOGGLE_ICON) &&
      (defn->title_flags & IT) == IT)
  {
    clone_icon(&defn->title_data);
  }

  /* icons */

  for (i = 0; i < defn->icon_count; i++)
  {
    wimp_icon *icon;

    icon = &defn->icons[i];

    if ((icon->flags & IT) != IT)
      continue; /* not indirected + text */

    clone_icon(&icon->data);
  }

  EC(xwimp_create_window((wimp_window *) &defn->visible, &new_w));
  if (new_w == 0)
  {
    /* FIXME: free() everything */
    return 0; /* error */
  }

  free(defn);

  return new_w;
}

/* ----------------------------------------------------------------------- */

static void delete_cloned_icon(wimp_icon_data *data)
{
  char *p;

  p = data->indirected_text.text;
  if (p != NULL && p != (char *) -1)
    free(p);

  p = data->indirected_text.validation;
  if (p != NULL && p != (char *) -1)
    free(p);
}

/* Dispose of window plus all indirected icon data.
 * Will probably cock up if window wasn't cloned. */
void window_delete_cloned(wimp_w w)
{
  wimp_window_info  info;
  wimp_window_info *defn;
  int               i;

  info.w = w;
  wimp_get_window_info_header_only(&info);

  /* this is one icon larger than it strictly needs to be */
  defn = malloc(sizeof(*defn) + info.icon_count * sizeof(wimp_icon));
  if (defn == NULL)
    error__fatal_oom();

  defn->w = w;
  wimp_get_window_info(defn);

  /* title */

  if ((defn->flags & wimp_WINDOW_TOGGLE_ICON) &&
      (defn->title_flags & IT) == IT)
  {
    delete_cloned_icon(&defn->title_data);
  }

  /* icons */

  for (i = 0; i < defn->icon_count; i++)
  {
    wimp_icon *icon;

    icon = &defn->icons[i];

    if ((icon->flags & IT) != IT)
      continue; /* not indirected + text */

    delete_cloned_icon(&icon->data);
  }

  free(defn);

  wimp_delete_window(w);
}
