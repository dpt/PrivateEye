/* --------------------------------------------------------------------------
 *    Name: os.h
 * Purpose: OS
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

#define osversion_200     (0xA1)
#define osversion_201     (0xA2)
#define osversion_300     (0xA3)
#define osversion_31x     (0xA4)
#define osversion_350     (0xA5)
#define osversion_360     (0xA6)
#define osversion_37x     (0xA7)
#define osversion_380_40X (0xA8)
#define osversion_4XX     (0xA9) /* also RISC OS "SIX" */
#define osversion_5       (0xAA)

#define os_version() \
    osbyte1(osbyte_IN_KEY, 0, 0xFF)

/* Returns the specified module's version number as BCD, or -1 if not found.
 */
int get_module_version(const char *module_name);

#endif /* APPENGINE_OS_H */
