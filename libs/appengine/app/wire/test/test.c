
#include <stdio.h>
#include <stdlib.h>

#ifdef FORTIFY
#include "fortify/fortify.h"
#endif

#include "appengine/types.h"
#include "appengine/base/errors.h"

#include "appengine/app/wire.h"

/* ----------------------------------------------------------------------- */

typedef struct client
{
  const char *name;
  wire_id     id;
}
client_t;

/* ----------------------------------------------------------------------- */

static result_t client_callback(const wire_message_t *message, void *opaque)
{
  client_t *client = opaque;

  printf("client %p/\"%s\" informed of event\n", opaque, client->name);
  printf("event was %x, payload was %p\n", message->event, message->payload);

  return result_OK;
}

static result_t create_wire_client(const char           *name,
                                   wire_register_flags   flags,
                                   client_t            **new_client)
{
  result_t  err;
  client_t *client;

  *new_client = NULL;

  client = malloc(sizeof(*client));
  if (client == NULL)
    return result_OOM;

  client->name = name;

  printf("register client %p/\"%s\"\n", (void *) client, name);

  err = wire_register(flags, client_callback, client, &client->id);
  if (err)
  {
    free(client);
    return err;
  }

  printf("new client's wire id is %x\n", client->id);

  *new_client = client;

  return result_OK;
}

static void destroy_wire_client(client_t *doomed)
{
  printf("deregister client %p/\"%s\" with id of %x\n",
         (void *) doomed, doomed->name, doomed->id);

  wire_deregister(doomed->id);

  free(doomed);
}

/* ----------------------------------------------------------------------- */

int wire_test(void)
{
  result_t        err;
  client_t       *clientA, *clientB, *clientC;
  wire_message_t  message;

  err = wire_init();
  if (err)
    goto failure;

  err = create_wire_client("Fred", 0, &clientA);
  if (err)
    goto failure;

  err = create_wire_client("Jim", 0, &clientB);
  if (err)
    goto failure;

  err = create_wire_client("Sheila", 0, &clientC);
  if (err)
    goto failure;

  message.event   = 0x42;
  message.payload = NULL;
  wire_dispatch(&message);

  destroy_wire_client(clientA);
  destroy_wire_client(clientB);
  destroy_wire_client(clientC);

  wire_fin();

  return 0;


failure:

  printf("\n\n*** Error %lx\n", err);

  return 1;
}
