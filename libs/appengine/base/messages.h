/* --------------------------------------------------------------------------
 *    Name: Messages.h
 * Purpose: Declarations for the MessageTrans library
 *  Author: David Thomas
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_MESSAGES_H
#define APPENGINE_MESSAGES_H

#include <stddef.h>

/* Messages.c */

/*
 * DocumentMe
 */
extern void open_messages(const char *filename);

/*
 * DocumentMe
 */
extern void close_messages(void);

/*
 * DocumentMe
 */
extern const char *messages_lookup(const char *token,
                                   const char *parameter1,
                                   const char *parameter2,
                                   const char *parameter3,
                                   const char *parameter4);

/*
 * DocumentMe
 */
extern const char *messages_enumerate(const char *wildcarded_token,
                                      int *index);

/*
 * DocumentMe
 */
extern const char *message_direct(const char *token);

/*
 * DocumentMe
 */
extern char *get_messages_fd(void); /* icky */

/* 
 * Compatibility macros
 */
#define message(m)	    messages_lookup(m,NULL,NULL,NULL,NULL)
#define message0(m)	    messages_lookup(m,NULL,NULL,NULL,NULL)
#define message1(m,n)	    messages_lookup(m,   n,NULL,NULL,NULL)
#define message2(m,n,o)	    messages_lookup(m,   n,   o,NULL,NULL)
#define message3(m,n,o,p)   messages_lookup(m,   n,   o,   p,NULL)
#define message4(m,n,o,p,q) messages_lookup(m,   n,   o,   p,   q)

#endif /* APPENGINE_MESSAGES_H */
