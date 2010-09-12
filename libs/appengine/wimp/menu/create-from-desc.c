/* $Id: create-from-desc.c,v 1.4 2009-05-18 22:07:52 dpt Exp $ */

/* EXAMPLES
 *
 * "App, >Info, Choices..., Quit"
 *
 * "App, File { File, >Info, >Source, >Histogram }, >Save, Edit { Edit, Copy }, Effect { Effect, Process...  }, Image { Image, Transform { Transform, Rotate { Rotate, 90deg, 180deg, 270deg }, Flip { Flip, Horizontal, Vertical } } }, >Scale"
 *
 * "App, %s '%s' { %s, >Copy, >Rename, Delete }"
 *       +---|-----+--- e.g. both are "File"
 *           +--------- e.g. the filename
 *
 * TODO
 *
 * - Instead of str_dup, keep a single buffer of all indirect text.
 * - Let the Parser return a Title state instead of keeping it with the Name
 *   state.
 * - Merge Sep/DashSep.
 *
 */

#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "fortify/fortify.h"

#include "oslib/wimp.h"

#include "appengine/base/appengine.h"
#include "appengine/base/errors.h"
#include "appengine/base/messages.h"
#include "appengine/base/strings.h"
#include "appengine/wimp/menu.h"

#ifndef NDEBUG
#define PRINTERROR(x) printf("Error: " x "\n")
#else
#define PRINTERROR(x)
#endif

enum { TextBufferSize = 64 };

#define MENUWIDTH(x) (((x) + 1) * 16)

static int title_is_indirected = 0;
static int menu_width          = 0;

static error setup_title(wimp_menu *m, const char *text)
{
  int text_len;

  text_len = strlen(text);
  if (text_len < 12)
  {
    /* make menu title */
    strcpy(m->title_data.text, text);
  }
  else
  {
    /* indirected */
    m->title_data.indirected_text.text = str_dup(text);
    if (m->title_data.indirected_text.text == NULL)
      return error_OOM;

// 'reserved' apparently
//    m->title_data.indirected_text.validation = NULL;
//    m->title_data.indirected_text.size = text_len + 1;

    /* first menu entry should set flags bit 8 */
    title_is_indirected = 1;
  }

  m->title_fg    = 7; /* black */
  m->title_bg    = 2; /* grey */
  m->work_fg     = 7; /* black */
  m->work_bg     = 0; /* white */
  m->width  = MENUWIDTH(text_len);
  m->height = 44;
  m->gap         = 0;

  menu_width = m->width;

  return error_OK;
}

static void setup_entry(wimp_menu *m, int index, const char *text)
{
  int           text_len;
  wimp_menu_entry *e;

  e = &m->entries[index];

  text_len = strlen(text);
  if (text_len < 12)
  {
    /* ordinary */
    e->menu_flags = title_is_indirected << 8;
    e->sub_menu   = NULL;
    e->icon_flags = 0x07000021; /* non-indirected */
    strcpy(e->data.text, text);
  }
  else
  {
    /* indirected */
    e->menu_flags = title_is_indirected << 8;
    e->sub_menu   = NULL;
    e->icon_flags = 0x07000121; /* indirected */
    e->data.indirected_text.text       = str_dup(text);
    e->data.indirected_text.validation = NULL;
    e->data.indirected_text.size       = text_len + 1;
  }

  title_is_indirected = 0;

  if (MENUWIDTH(text_len) > menu_width)
    menu_width = MENUWIDTH(text_len);
}

static void terminate_menu(wimp_menu *m)
{
  m->width = menu_width;
}

/* ----------------------------------------------------------------------- */

typedef enum { Name, Sep, DashSep, Push, Pop, End, Error } Token;

enum
{
  Tick    = 1 << 0,
  Shade   = 1 << 1,
  SubMenu = 1 << 2
};
typedef unsigned int EntryFlags;

struct Parser
{
  const char  *p;           /* points to desc string */
  const char  *start, *end; /* end is exclusive */
  EntryFlags   flags;
};
typedef struct Parser Parser;

/* ----------------------------------------------------------------------- */

static void initparser(Parser *parser, const char *desc)
{
  parser->p = desc;
}

static void getopt(Parser *parser)
{
  const char *p;

  parser->flags = 0;

  p = parser->p;

  for (;;)
  {
    int c;

    c = *p;

         if (c == '!') parser->flags |= Tick;
    else if (c == '>') parser->flags |= SubMenu;
    else if (c == '~') parser->flags |= Shade;
    else { parser->p = p; return; }

    p++;
  }
}

static int isdelim(int c)
{
  return c == ',' || c == '|' || c == '{' || c == '}' || c == '\0';
}

static void getname(Parser *parser)
{
  const char *p;

  getopt(parser); /* eat any options */

  p = parser->p;

  parser->start = p;

  while (!isdelim(*p))
    p++;

  parser->end = p; /* points after */

  parser->p = p;
}

static Token getnext(Parser *parser)
{
  while (isspace(*parser->p)) /* skip spaces */
    parser->p++;

  if (!isdelim(*parser->p))
  {
    getname(parser);
    if (parser->start == parser->end)
      return Error; /* no valid name was found */

    return Name;
  }

  switch (*parser->p++)
  {
  case ',': return Sep;
  case '|': return DashSep;
  case '{': return Push;
  case '}': return Pop;
  }

  assert(parser->p[-1] == '\0');

  return End;
}

/* ----------------------------------------------------------------------- */

typedef struct MakeMenus MakeMenus;

typedef struct
{
  error       (*add)(MakeMenus *maker, const char *text);
  wimp_menu     *base;
  int           cur;  /* current entry */
  int           max;  /* space allocated */
}
GrowableMenu;

struct MakeMenus
{
  Parser        parser;

  int           stack[8];  /* matches Wimp menu depth limit */
  int          *sp;

  GrowableMenu  menus[32]; /* limits the total number of menus we can create */
  int           cur;       /* current slot in menus[] array (the menu we're building) */
  int           nextfree;  /* next available menu slot */
};

#define MENUSZ(n) (sizeof(wimp_menu) + sizeof(wimp_menu_entry) * (n - 1))

static error addentry(MakeMenus *maker, const char *text)
{
  GrowableMenu    *g;
  wimp_menu_flags  flags;

  g = maker->menus + maker->cur;

  if (g->cur == g->max)
  {
    wimp_menu *base;
    int       max;

    max = g->max * 2;
    if (max < 2)
        max = 2;

    base = realloc(g->base, MENUSZ(max));
    if (base == NULL)
      return error_OOM;

    g->base = base;
    g->max  = max;
  }

  setup_entry(g->base, g->cur, text);

  /* menu flags */

  flags = 0;
  if (maker->parser.flags & Tick)
    flags |= wimp_MENU_TICKED;
  if (maker->parser.flags & SubMenu)
    flags |= wimp_MENU_GIVE_WARNING;
  menu_set_menu_flags(g->base, g->cur, flags, flags);

  /* icon flags */

  if (maker->parser.flags & Shade)
    menu_set_icon_flags(g->base, g->cur, wimp_ICON_SHADED, wimp_ICON_SHADED);

  g->cur++;

  return error_OK;
}

static error addtitle(MakeMenus *maker, const char *text)
{
  error         err;
  GrowableMenu *g;

  g = maker->menus + maker->cur;

  err = setup_title(g->base, text);
  if (err)
    return err;

  g->add = addentry;

  return error_OK;
}

static error newmenu(MakeMenus *maker)
{
  enum { InitialEntries = 8 };

  wimp_menu     *m;
  int           i;
  GrowableMenu *g;

  m = malloc(MENUSZ(InitialEntries));
  if (m == NULL)
    return error_OOM;

  i = maker->nextfree++;

  g = maker->menus + i;

  g->add   = addtitle;
  g->base  = m;
  g->cur   = 0;
  g->max   = InitialEntries;

  maker->cur = i;

  return error_OK;
}

static void setflags(MakeMenus *maker, unsigned int flags)
{
  GrowableMenu *g;

  g = maker->menus + maker->cur;

  /* set *previous* emitted entry */
  menu_set_menu_flags(g->base, g->cur - 1, flags, flags);
}

static void setsubmenu(MakeMenus *maker, void *submenu)
{
  GrowableMenu *g;

  g = maker->menus + maker->cur;

  /* set *previous* emitted entry */
  menu_set_submenu(g->base, g->cur - 1, submenu);
}

static error menufinished(MakeMenus *maker)
{
  GrowableMenu *g;
  wimp_menu     *m;

  setflags(maker, wimp_MENU_LAST);

  g = maker->menus + maker->cur;

  terminate_menu(g->base);

  /* realloc to fit exactly */
  m = realloc(g->base, MENUSZ(g->cur));
  if (m == NULL)
    return error_OOM;

  g->base = m;

  return error_OK;
}

/* ### start/end cases are similar to the push/pop cases -- merge */

static error makemenus(MakeMenus *maker, va_list ap)
{
  error err;

  maker->sp = maker->stack; /* initially empty */

  maker->nextfree = 0;
  err = newmenu(maker);
  if (err)
    return err;

  for (;;)
  {
    switch (getnext(&maker->parser))
    {
      case Name:
        {
          char        text[TextBufferSize]; /* Careful Now */
          char       *q;
          const char *p;

          /* copy the name across, substituting %s with strings from the
           * va_list */

          q = text;
          for (p = maker->parser.start; p != maker->parser.end; p++)
          {
            int c = *p;

            /* check specifically for "%s" */

            if (c == '%' && (p + 1) < maker->parser.end && p[1] == 's')
            {
              const char *substr;
              int         len;

              substr = va_arg(ap, const char *);
              len    = strlen(substr);

              memcpy(q, substr, len);
              q += len;

              p++; /* skip two chars */
            }
            else
            {
              *q++ = c;
            }
          }

          *q++ = '\0';

          err = maker->menus[maker->cur].add(maker, text);
          if (err)
            return err;

          if (maker->parser.flags & SubMenu)
            setsubmenu(maker, va_arg(ap, void *));
        }
        break;

      case Sep:
        break;

      case DashSep:
        /* set dotted separator flag */
        setflags(maker, wimp_MENU_SEPARATE);
        break;

      case Push:
        if (maker->sp >= maker->stack + 8)
        {
          PRINTERROR("out of stack space / menu too deep");
          return error_OK;
        }

        *maker->sp++ = maker->cur;

        err = newmenu(maker); /* init next menu */
        if (err)
          return err;

        break;

      case Pop:
        {
          int popped;

          if (maker->sp == maker->stack)
          {
            PRINTERROR("pop without previous push");
            return error_OK;
          }

          err = menufinished(maker);
          if (err)
            return err;

          popped = maker->cur; /* need this in a moment */

          /* move back to parent menu */
          maker->cur = *(--maker->sp); /* sp points to empty so pre-decrement */

          /* set submenu ptr of parent menu entry */
          setsubmenu(maker, maker->menus[popped].base);
        }
        break;

      case End:
        err = menufinished(maker);
        return err;

      case Error:
        PRINTERROR("unhandled error");
        return error_OK;

      default:
        assert(0);
        break;
    }
  }

  return error_OK;
}

wimp_menu *menu_create_from_desc(const char *desc, ...)
{
  error err;
  va_list ap;
  MakeMenus maker;

  /* VA_ARG(3) man page says: "You may pass a `va_list' object AP to a
   * subfunction, and use `va_arg' from the subfunction rather than from the
   * function actually declared with an ellipsis in the header; however, in
   * that case you may _only_ use `va_arg' from the subfunction. ANSI C does
   * not permit extracting successive values from a single variable-argument
   * list from different levels of the calling stack." */

  va_start(ap, desc);
  initparser(&maker.parser, desc);
  err = makemenus(&maker, ap);
  va_end(ap);

  if (err)
    return NULL;

  return maker.menus[0].base;
}

/* ----------------------------------------------------------------------- */

/* |Flintstone>|Fred  |
 * |           |Wilma |
 * |Rubble    >|Barney|
 * |           |Betty |
 *
 * So (0,0,-1) is 'Fred'
 *    (1,1,-1) is 'Betty'
 *    (1,-1)   is 'Rubble'
 */

/* turn a wimp menu selection structure into a single token */
const char *menu_desc_name_from_sel(const char           *desc,
                                    const wimp_selection *sel)
{
  int            seldepth;
  int            depth;
  wimp_selection mysel;
  int            title;
  Parser         parser;

  for (seldepth = 0; sel->items[seldepth] != -1; seldepth++)
    ;

  depth = 0;
  mysel.items[depth] = -1;

  title = 1;

  initparser(&parser, desc);

  for (;;)
  {
    switch (getnext(&parser))
    {
      case Name:
        if (title)
        {
          title = 0;
        }
        else
        {
          mysel.items[depth]++;

          if (depth + 1 == seldepth &&
              memcmp(&mysel.items[0],
                     &sel->items[0],
                      seldepth * sizeof(sel->items[0])) == 0)
          {
            static char text[TextBufferSize]; /* Careful Now */
            size_t      l;

            l = parser.end - parser.start;
            if (l > sizeof(text) - 1)
              l = sizeof(text) - 1;

            memcpy(text, parser.start, l);
            text[l] = '\0';
            return text;
          }
        }
        break;

      case Sep:
      case DashSep:
        break;

      case Push:
        depth++;
        mysel.items[depth] = -1;
        title = 1;
        break;

      case Pop:
        mysel.items[depth] = -1;
        depth--;
        title = 0;
        break;

      case End:
      case Error:
      default:
        return NULL;
    }
  }

  return NULL;
}
