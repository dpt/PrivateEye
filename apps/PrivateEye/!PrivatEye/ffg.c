/* --------------------------------------------------------------------------
 *    Name: ffg.c
 * Purpose: Computer Concepts Foreign Format Graphics converters interface
 * ----------------------------------------------------------------------- */

/* TODO
 *
 * There's no way to prioritise which converters are chosen, or which formats
 * are available. If a transloader offers both sprite and DrawFile output,
 * we'll choose the first in the list.
 */

/* TransTIFF+ gives corrupt reply refs... I can't use them. */

#include "kernel.h"

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fortify/fortify.h"

#include "oslib/types.h"
#include "oslib/os.h"
#include "oslib/osfile.h"
#include "oslib/wimp.h"

#include "datastruct/list.h"

#include "appengine/types.h"
#include "appengine/wimp/event.h"

#include "globals.h"

#include "ffg.h"

static list_t list_anchor = { NULL };
static int    my_ref;
static wimp_t ffg_task;

/* ----------------------------------------------------------------------- */

typedef struct Translator
{
  list_t list;
  char   unique_name[16];
  char  *cmd;
  int    ntypes;
  struct
  {
    bits from, to;
  }
  types[1];
}
Translator;

/* ----------------------------------------------------------------------- */

typedef osbool (ffg__loadable)(bits file_type);

/* ----------------------------------------------------------------------- */

static void read_translators(const char    *wildcard,
                             ffg__loadable *loadable_fn)
{
  enum { MaxTypes = 8 }; /* FIXME: Hard limit. */

  int context;

  context = 0;

  for (;;)
  {
    char         value[256];
    int          used;
    os_var_type  var_type;
    Translator  *ffg;
    const char  *cmd;
    size_t       cmdlen;
    const char  *dollar;
    int          i;
    const char  *p;
    os_error    *e;

    e = xos_read_var_val(wildcard,
                         value,
                         sizeof(value),
                         context,
                         os_VARTYPE_STRING,
                        &used,
                        &context,
                        &var_type);

    /* XOS_ReadVarVal returns context == 0 *and* VS, but OSLib's veneer jumps
     * immediately on seeing the returned VS, never updating the returned
     * variables. So we must check the error, not 'context'. */

    if (e != NULL && e->errnum == error_VAR_CANT_FIND)
      break; /* no more system variables */

    value[used++] = '\0';

    ffg = malloc(offsetof(Translator, types) +
                 MaxTypes * sizeof(ffg->types[0]));
    if (ffg == NULL)
      return; /* OOM */

    /* get the server command */

    cmd = strstr(value, "-R ");
    if (cmd == NULL)
    {
      free(ffg);
      continue; /* not found: next var please */
    }

    cmd += 3;

    for (cmdlen = 0; cmd[cmdlen] != '\0'; cmdlen++)
      if (isspace(cmd[cmdlen]))
        break;

    ffg->cmd = malloc(cmdlen + 3);
    if (ffg->cmd == NULL)
    {
      free(ffg);
      return; /* OOM */
    }

    strcpy(ffg->cmd, "<");
    strncat(ffg->cmd, cmd, cmdlen);
    strcat(ffg->cmd, ">");

    /* copy unique name */

    /* The context returned from OS_ReadVarVal is really a pointer to the
     * read variable's name. Hopefully it's always zero terminated. */

    p = (const char *) context;
    dollar = strchr(p, '$');
    assert(dollar != NULL);

    strncpy(ffg->unique_name, dollar + 1, 16);
    ffg->unique_name[15] = '\0';

    i = 0;

    for (p = value; p + 2 < value + used; )
    {
      if (p[0] == '-' && p[1] == 'T' && p[2] == ' ')
      {
        int  c;
        bits from, to;

        p += 3;

        c = sscanf(p, "&%x &%x", &from, &to); /* FIXME: &s are optional */
        if (c != 2)
          continue;

        if (!loadable_fn(to)) /* is 'to' something we understand? */
          continue;

        if (i < MaxTypes)
        {
          ffg->types[i].from = from;
          ffg->types[i].to   = to;
          i++;
        }
        else
        {
          /* FIXME: Need a bigger block */
        }
      }
      else
      {
        p++;
      }
    }

    if (i == 0)
    {
      /* didn't add any types - discard the block */
      free(ffg->cmd);
      free(ffg);
      continue;
    }

    ffg->ntypes = i;

    list_add_to_head(&list_anchor, &ffg->list);
  }
}

typedef union
{
  bits              file_type;
  const Translator *t;
}
get_converter_args;

static int get_converter_callback(list_t *list, void *opaque)
{
  const Translator   *t    = (const Translator *) list;
  get_converter_args *args = opaque;
  int                 i;

  for (i = 0; i < t->ntypes; i++)
    if (t->types[i].from == args->file_type)
    {
      args->t = t;
      return -1; /* found */
    }

  return 0; /* not found */
}

static const Translator *get_converter(bits src_file_type)
{
  get_converter_args args;

  args.file_type = src_file_type;

  return list_walk(&list_anchor, get_converter_callback, &args) ?
         args.t : NULL;
}

/* ----------------------------------------------------------------------- */

static event_message_handler message_translate_ffg_ack,
                             messageack_translate_ffg;

void ffg_initialise(ffg__loadable *loadable_fn)
{
  static const char *vars[] =
  {
    "FFGServer$*"
    /* FFXServer$* and FileSwopper$* also exist but not quite sure about
     * these yet */
  };

  int i;

  for (i = 0; i < NELEMS(vars); i++)
    read_translators(vars[i], loadable_fn);

  event_register_message_handler(message_TRANSLATE_FFG_ACK,
                                 event_ANY_WINDOW,
                                 event_ANY_ICON,
                                 message_translate_ffg_ack,
                                 NULL);

  event_register_messageack_handler(message_TRANSLATE_FFG,
                                    event_ANY_WINDOW,
                                    event_ANY_ICON,
                                    messageack_translate_ffg,
                                    NULL);
}

static int ffg__finalise_callback(list_t *list, void *opaque)
{
  Translator *t = (Translator *) list;

  NOT_USED(opaque);

  free(t->cmd);
  free(t);

  return 0;
}

void ffg_finalise(void)
{
  event_deregister_messageack_handler(message_TRANSLATE_FFG,
                                      event_ANY_WINDOW,
                                      event_ANY_ICON,
                                      messageack_translate_ffg,
                                      NULL);

  event_deregister_message_handler(message_TRANSLATE_FFG_ACK,
                                   event_ANY_WINDOW,
                                   event_ANY_ICON,
                                   message_translate_ffg_ack,
                                   NULL);

  list_walk(&list_anchor, ffg__finalise_callback, NULL);
}

/* ----------------------------------------------------------------------- */

osbool ffg_is_loadable(bits src_file_type)
{
  return get_converter(src_file_type) != NULL;
}

/* ----------------------------------------------------------------------- */

typedef struct
{
  wimp_MESSAGE_HEADER_MEMBERS
  union
  {
    struct
    {
      wimp_w   w;
      wimp_i   i;
      os_coord pos;
      int      dataload_your_ref;
      bits     src_file_type;
      bits     dst_file_type;
      char     unique_name[16];
      char     params[40];
      char     file_name[152];
    }
    translate_ffg;
  }
  data;
}
wimp_message_ffg;

osbool ffg_convert(const wimp_message *message)
{
  wimp_message_ffg  msg;
  const Translator *server;
  int               i;

  server = get_converter(message->data.data_xfer.file_type);
  if (server == NULL)
    return 1;

  wimp_start_task(server->cmd);

  msg.data.translate_ffg.w     = message->data.data_xfer.w;
  msg.data.translate_ffg.i     = message->data.data_xfer.i;
  msg.data.translate_ffg.pos.x = message->data.data_xfer.pos.x;
  msg.data.translate_ffg.pos.y = message->data.data_xfer.pos.y;
  msg.data.translate_ffg.dataload_your_ref = message->your_ref;

  /* need src/dst types at this point so have to do this *repeated work* */

  for (i = 0; i < server->ntypes; i++)
    if (server->types[i].from == message->data.data_xfer.file_type)
      break;

  msg.data.translate_ffg.src_file_type = server->types[i].from;
  msg.data.translate_ffg.dst_file_type = server->types[i].to;

  /* unique name and params must be zero-filled */
  memset(&msg.data.translate_ffg.unique_name, 0, 16 + 40);

  strcpy(msg.data.translate_ffg.unique_name, server->unique_name);

  strncpy(msg.data.translate_ffg.file_name,
          message->data.data_xfer.file_name,
          sizeof(msg.data.translate_ffg.file_name));
  msg.data.translate_ffg.file_name[151] = '\0'; /* just in case */

  msg.size = (offsetof(wimp_message_ffg, data.translate_ffg.file_name) +
              strlen(msg.data.translate_ffg.file_name) + 1 + 3) & ~3;
  /* msg.sender and msg.my_ref set for me on send */
  msg.your_ref = 0;
  msg.action   = message_TRANSLATE_FFG;

  wimp_send_message(wimp_USER_MESSAGE_RECORDED,
  (wimp_message *) &msg,
                    wimp_BROADCAST);

  my_ref = msg.my_ref;

  ffg_task = 0;

  return 0;
}

/* ----------------------------------------------------------------------- */

/* message_TRANSLATE_FFG_ACK was received. */
static int message_translate_ffg_ack(wimp_message *message, void *handle)
{
  NOT_USED(handle);

  ffg_task = message->sender;

  return event_HANDLED;
}

/* An acknowlegement of message_TRANSLATE_FFG was received. */
static int messageack_translate_ffg(wimp_message *message, void *handle)
{
  NOT_USED(message);
  NOT_USED(handle);

  /* " The wimp will return Message_TranslateFFG if the translator failed to
   *   start up. It should remove the file if offset 36 is non-zero and
   *   report an error 'translator died'. "
   */

  oserror_report(12345, "error.ffg.died");

  return event_HANDLED;
}

/* ----------------------------------------------------------------------- */

int ffg_apposite(const wimp_message *message)
{
  return message->sender == ffg_task;
}

void ffg_complete(const wimp_message *message)
{
  /* Transloaders don't always set a correct file type of the file they've
   * converted. Set it here to the value in the messsage so that viewer_load
   * can work. */
  xosfile_set_type(message->data.data_xfer.file_name,
                   message->data.data_xfer.file_type);

  ffg_task = 0;
}
