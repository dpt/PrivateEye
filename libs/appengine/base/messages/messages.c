
#include "swis.h"

#include <stdlib.h>
#include <string.h>

#include "oslib/os.h"

/* #include "MemCheck:MemCheck.h" */

#include "appengine/base/appengine.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/oserror.h"

/* ----------------------------------------------------------------------- */

/*
 * messages.fd is NULL initially so that calls to messages_lookup() which
 * happen before open_messages has been called will use the global messages.
 */

static struct
{
  char  *fd;
  char  *data; /* NULL implies used directly */
  char  *buf;
  size_t bufsz;
}
messages = { NULL, NULL, NULL, 0 };

/* ----------------------------------------------------------------------- */

static void expandbuf(void)
{
  char  *buf;
  size_t bufsz;

  bufsz = messages.bufsz * 2;
  buf   = realloc(messages.buf, bufsz);
  if (buf == NULL)
    error__fatal_oom();

  messages.buf   = buf;
  messages.bufsz = bufsz;
}

/* ----------------------------------------------------------------------- */

/*
 * Open the specified Messages file, using it directly if possible.
 */

void open_messages(const char *filename)
{
  int flags;
  int size;
  int blocksize;
  os_error *e;

  if (messages.fd != NULL)
    return;

  messages.bufsz = 64;
  messages.buf   = malloc(messages.bufsz);
  if (messages.buf == NULL)
    goto NoMem;

 /*
  * Using _swi in preference to _swix to make any errors fatal.
  */

  _swi(MessageTrans_FileInfo, _IN(1)|_OUT(0)|_OUT(2),
       filename,
      &flags,
      &size);

  if (flags & 1)
  {
    /* File is in memory - use directly */
    messages.data = NULL;
  }
  else
  {
    /* File is not in memory - buffer it */
    messages.data = malloc(size);
    if (messages.data == NULL)
      goto NoMem;
  }

  blocksize = 16 + strlen(filename) + 1;

  _swi(OS_Module, _IN(0)|_IN(3)|_OUT(2), 6, blocksize, &messages.fd);

  /* MemCheck_RegisterMiscBlock(messages.fd, blocksize); */

  strcpy(messages.fd + 16, filename);

  e = EC(_swix(MessageTrans_OpenFile, _INR(0,2),
               messages.fd,
               messages.fd + 16,
               messages.data));

  if (e != NULL)
  {
    /* MemCheck_UnRegisterMiscBlock(messages.fd); */
    _swi(OS_Module, _IN(0)|_IN(2), 7, messages.fd);
    messages.fd = NULL;

    free(messages.data);
    free(messages.buf);

    _swi(OS_GenerateError, _IN(0), e);
  }

  return;

NoMem:
  free(messages.buf);
  error__fatal_oom();
}

/*
 * Close the current Messages file.
 */

void close_messages(void)
{
  if (messages.fd == NULL)
    return;

  _swi(MessageTrans_CloseFile, _IN(0), messages.fd);

  /* MemCheck_UnRegisterMiscBlock(messages.fd); */
  _swi(OS_Module, _IN(0)|_IN(2), 7, messages.fd);
  messages.fd = NULL;

  free(messages.data);
}

/*
 * Function to look up a message with up to 4 parameters.
 */

const char *messages_lookup(const char *token,
                            const char *parameter1,
                            const char *parameter2,
                            const char *parameter3,
                            const char *parameter4)
{
  int length;
  int buftoosmall;

  do
  {
    if (_swix(MessageTrans_Lookup, _INR(0,7)|_OUT(3),
              messages.fd,
              token,
              messages.buf,
              messages.bufsz,
              parameter1, parameter2, parameter3, parameter4,
             &length) != NULL)
    {
      length = 0; /* error: return an empty string */
      break;
    }

    /* if we've maxed out the buffer, then assume we need to realloc */

    buftoosmall = (length == messages.bufsz - 1);

    if (buftoosmall)
      expandbuf();
  }
  while (buftoosmall);

  messages.buf[length] = '\0';

  return messages.buf;
}

/*
 * Function to enumerate messages.
 */

const char *messages_enumerate(const char *wildcarded_token, int *index)
{
  int more;
  int length;
  int buftoosmall;

  do
  {
    EC(_swix(MessageTrans_EnumerateTokens, _INR(0,4)|_OUTR(2,4),
             messages.fd,
             wildcarded_token,
             messages.buf,
             messages.bufsz,
            *index,
            &more,
            &length,
             index));

    if (more == 0)
      return NULL; /* no more */

    /* if we've maxed out the buffer, then assume we need to realloc */

    buftoosmall = (length == messages.bufsz - 1);

    if (buftoosmall)
      expandbuf();
  }
  while (buftoosmall);

  messages.buf[length] = '\0';

  return messages.buf;
}

/*
 * Function to lookup a message and return a pointer direct to the Messages
 * file.
 *
 * Note that messages looked up this manner will be LF terminated, so a NUL
 * character must be added to the specified message in the Messages file.
 */

const char *message_direct(const char *token)
{
  const char *message;

  if (EC(_swix(MessageTrans_Lookup, _INR(0,2)|_OUT(2),
               messages.fd,
               token,
               0,
              &message)) != NULL)
  {
    return ""; /* return an empty string */
  }

  return message;
}


/*
 * icky - need for use with MessageTrans_MakeMenus
 */

char *get_messages_fd(void)
{
  return messages.fd;
}
