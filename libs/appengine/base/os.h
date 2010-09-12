/* --------------------------------------------------------------------------
 *    Name: os.h
 * Purpose: OS
 * Version: $Id: os.h,v 1.1 2009-05-18 22:07:49 dpt Exp $
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_OS_H
#define APPENGINE_OS_H

#define INKEY_SHIFT (0 /* Shift */ ^ 0x80)
#define INKEY_CTRL  (1 /* Ctrl  */ ^ 0x80)
#define INKEY_ALT   (2 /* Alt   */ ^ 0x80)

/* Tests for keypresses */
int inkey(int key);

/* Sounds the system beep */
void beep(void);

#endif /* APPENGINE_OS_H */
