/* --------------------------------------------------------------------------
 *    Name: wire.h
 * Purpose: Wimp-like messaging for avoiding direct coupling between modules
 * ----------------------------------------------------------------------- */

/* Wire is a very simple callback system. It is called 'wire' as my mental
 * picture of it is a thread which joins otherwise uncoupled modules. Clients
 * register an interest and receive a callback whenever wire_dispatch() is
 * called. This allows clients to communicate without direct coupling in the
 * same manner as Wimp apps do using messages. */

/* This is similar to image observers. */

// TODO
//
// error handling...
// do modules need ids?
// should we add point-to-point messages?
// add service oriented stuff - clients and servers?
// add equivalents of the data transfer protocol?

#ifndef APPENGINE_WIRE_H
#define APPENGINE_WIRE_H

#include "appengine/base/errors.h"

/* ----------------------------------------------------------------------- */

result_t wire_init(void);
void wire_fin(void);

/* ----------------------------------------------------------------------- */

/* A wire client ID */
typedef unsigned int wire_id;

/* A wire message */
typedef struct wire_message wire_message_t;

/* Flags passed to wire_register */
typedef unsigned int wire_register_flags;

// These are just sketchy ideas for future features.
// #define wire_register_PRIORITY_MASK (3u << 0) // 0..3 low..high priority
//#define wire_register_CATEGORIES_MASK (0xff << 2) // call these channels?

typedef result_t (wire_callback)(const wire_message_t *message, void *opaque);

result_t wire_register(wire_register_flags  flags,
                       wire_callback       *cb,
                       void                *opaque,
                       wire_id             *id);
void wire_deregister(wire_id id);

/* ----------------------------------------------------------------------- */

/* Should I expose the contents of the wire_message and its associated
 * wire_event definitions here, or should they be defined by convention
 * between cooperating modules? */

typedef unsigned int wire_event;

/* Request to declare keymaps */
#define wire_event_DECLARE_KEYMAP ((wire_event) 0u)

/* ----------------------------------------------------------------------- */

struct wire_message
{
  wire_event event;
  void      *payload;
};

result_t wire_dispatch(const wire_message_t *message);

/* ----------------------------------------------------------------------- */

#endif /* APPENGINE_WIRE_H */
