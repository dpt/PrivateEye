/* wire.c */

#include <stddef.h>

#include "fortify/fortify.h"

#include "oslib/types.h"

#include "datastruct/list.h"

#include "appengine/types.h"
#include "appengine/base/errors.h"

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

result_t wire_init(void)
{
  list_init(&LOCALS.anchor);

  return result_OK;
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

  list_walk(&LOCALS.anchor, (list_walk_callback_t *) fin_cb, NULL);
}

/* ----------------------------------------------------------------------- */

result_t wire_dispatch(const wire_message_t *message)
{
  result_t          err;
  wire_client_t *e;
  wire_client_t *next;

  for (e = (wire_client_t *) LOCALS.anchor.next; e != NULL; e = next)
  {
    next = (wire_client_t *) e->list.next;
    if ((err = e->cb(message, e->opaque)) != result_OK)
      return err;
  }

  return result_OK;
}

/* ----------------------------------------------------------------------- */

result_t wire_register(wire_register_flags  flags,
                       wire_callback       *cb,
                       void                *opaque,
                       wire_id             *id)
{
  wire_client_t *client;

  client = malloc(sizeof(*client));
  if (client == NULL)
    return result_OOM;

  client->flags  = flags;
  client->cb     = cb;
  client->opaque = opaque;

  list_add_to_head(&LOCALS.anchor, &client->list);

  if (id)
    *id = (unsigned int) client;

  return result_OK;
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
