/* wire.c */

#include <stddef.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"
#include "appengine/datastruct/list.h"

#include "appengine/app/wire.h"

/* ----------------------------------------------------------------------- */

typedef struct wire_client
{
  list_t               list;
  wire_register_flags  flags;
  wire_callback       *cb;
  void                *opaque;
}
wire_client_t;

/* ----------------------------------------------------------------------- */

static struct
{
  list_t anchor;
}
LOCALS;

/* ----------------------------------------------------------------------- */

error wire_init(void)
{
  list_init(&LOCALS.anchor);

  return error_OK;
}

static int fin_cb(wire_client_t *doomed, void *opaque)
{
  NOT_USED(opaque);

  list_remove(&LOCALS.anchor, &doomed->list);

  free(doomed);

  return 0;
}

void wire_fin(void)
{
  /* inefficient: we walk list twice */

  list_walk(&LOCALS.anchor, (list_walk_callback *) fin_cb, NULL);
}

/* ----------------------------------------------------------------------- */

error wire_dispatch(const wire_message_t *message)
{
  error          err;
  wire_client_t *e;
  wire_client_t *next;

  for (e = (wire_client_t *) LOCALS.anchor.next; e != NULL; e = next)
  {
    next = (wire_client_t *) e->list.next;
    if ((err = e->cb(message, e->opaque)) != error_OK)
      return err;
  }

  return error_OK;
}

/* ----------------------------------------------------------------------- */

error wire_register(wire_register_flags  flags,
                    wire_callback       *cb,
                    void                *opaque,
                    wire_id             *id)
{
  wire_client_t *client;

  client = malloc(sizeof(*client));
  if (client == NULL)
    return error_OOM;

  client->flags  = flags;
  client->cb     = cb;
  client->opaque = opaque;

  list_add_to_head(&LOCALS.anchor, &client->list);

  if (id)
    *id = (unsigned int) client;

  return error_OK;
}

void wire_deregister(wire_id id)
{
  wire_client_t *doomed;

  doomed = (wire_client_t *) id;
  if (doomed == NULL)
    return;

  list_remove(&LOCALS.anchor, &doomed->list);

  free(doomed);
}
