/* --------------------------------------------------------------------------
 *    Name: match.c
 * Purpose: Font matching
 *  Author: David Thomas
 * Version: $Id: match.c,v 1.2 2010-01-13 18:08:40 dpt Exp $
 * ----------------------------------------------------------------------- */

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oslib/font.h"

#include "appengine/types.h"
#include "appengine/vdu/font.h"
#include "appengine/base/strings.h"

#define DBUG(args)

typedef struct nameattr
{
  char        name[11];
  int         mask;
  font__attrs value;
}
nameattr;

/* Sorted by name for binary searching. */
/* Weights are taken from TTF/OTF OS/2 table usWeightClass. */
static const nameattr map[] =
{
  { "Black",      font__WEIGHT_MASK,  font__WEIGHT_BLACK      },
  { "Bold",       font__WEIGHT_MASK,  font__WEIGHT_BOLD       },
  { "Book",       font__WEIGHT_MASK,  font__WEIGHT_400        },
  { "Comp",       font__STRETCH_MASK, font__STRETCH_COND      },
  { "Cond",       font__STRETCH_MASK, font__STRETCH_COND      },
  { "Demi",       font__WEIGHT_MASK,  font__WEIGHT_600        },
  { "DemiBold",   font__WEIGHT_MASK,  font__WEIGHT_600        },
  { "Expert",     font__VARIANT_MASK, font__VARIANT_EXPERT    },
  { "Extended",   font__STRETCH_MASK, font__STRETCH_EXT       },
  { "Extra",      font__WEIGHT_MASK,  font__WEIGHT_950        },
  { "ExtraBlack", font__WEIGHT_MASK,  font__WEIGHT_950        },
  { "ExtraBold",  font__WEIGHT_MASK,  font__WEIGHT_800        },
  { "ExtraComp",  font__STRETCH_MASK, font__STRETCH_EXTRACOND },
  { "ExtraLight", font__WEIGHT_MASK,  font__WEIGHT_200        },
  { "Heavy",      font__WEIGHT_MASK,  font__WEIGHT_900        },
  { "Italic",     font__STYLE_MASK,   font__STYLE_ITALIC      },
  { "Light",      font__WEIGHT_MASK,  font__WEIGHT_LIGHT      },
  { "Medium",     font__WEIGHT_MASK,  font__WEIGHT_500        },
  { "Normal",     font__WEIGHT_MASK,  font__WEIGHT_NORMAL     },
  { "OSF",        font__VARIANT_MASK, font__VARIANT_OSF       },
  { "Oblique",    font__STYLE_MASK,   font__STYLE_OBLIQUE     },
  { "Open",       font__VARIANT_MASK, font__VARIANT_OPEN      },
  { "Poster",     font__VARIANT_MASK, font__VARIANT_POSTER    },
  { "Regular",    font__WEIGHT_MASK,  font__WEIGHT_400        },
  { "Roman",      font__WEIGHT_MASK,  font__WEIGHT_400        },
  { "SemiBold",   font__WEIGHT_MASK,  font__WEIGHT_600        },
  { "Slanted",    font__STYLE_MASK,   font__STYLE_SLANTED     },
  { "SmallCaps",  font__VARIANT_MASK, font__VARIANT_SMALLCAPS },
  { "Thin",       font__WEIGHT_MASK,  font__WEIGHT_100        },
  { "Title",      font__VARIANT_MASK, font__VARIANT_TITLE     },
  { "Ultra",      font__WEIGHT_MASK,  font__WEIGHT_800        },
  { "UltraBlack", font__WEIGHT_MASK,  font__WEIGHT_950        },
  { "UltraBold",  font__WEIGHT_MASK,  font__WEIGHT_800        },
  { "UltraComp",  font__STRETCH_MASK, font__STRETCH_ULTRACOND },
  { "UltraLight", font__WEIGHT_MASK,  font__WEIGHT_200        },
};

/* Copy 'in' to 'out' zero-terminating all full stops and placing pointers
 * to each field in tokens[]. */
static int tokenise(const char *in, char *out, char *tokens[])
{
  int         i;
  const char *p;
  char       *q;

  /* e.g. Baskerville.SmallCaps.DemiBold.Italic */

  i = 0;
  for (p = in, q = out; *p != '\0'; p++)
  {
    int c = *p;

    if (c == '.')
    {
      *q++ = '\0'; /* terminate each token */
      tokens[i++] = q;
    }
    else
    {
      *q++ = c;
    }
  }

  *q++ = '\0';

  return i; /* return number of found tokens */
}

static int compattrs(const void *a, const void *b)
{
  const nameattr *ma = a;
  const nameattr *mb = b;

  return strcasecmp(ma->name, mb->name);
}

/* Compute the distance between two attrs. A smaller value is better (ie.
 * closer). */
static int score(font__attrs a, font__attrs b)
{
  return abs((a & font__WEIGHT_MASK)  - (b & font__WEIGHT_MASK))  +
         abs((a & font__STYLE_MASK)   - (b & font__STYLE_MASK))   +
         abs((a & font__STRETCH_MASK) - (b & font__STRETCH_MASK)) +
         abs((a & font__VARIANT_MASK) - (b & font__VARIANT_MASK));
}

/* Split a font name into family name and attrs.
 *
 * We consider the font name to be the leading part of the string up to the
 * first recognised token. ie. the "Goudy.OldStyle.Bold" family is
 * "Goudy.OldStyle".
 *
 * Foo.Bold.Bar        -> Foo
 * Foo.Bar.Bold        -> Foo.Bar
 * Foo.Bar.Baz.Oblique -> Foo.Bar.Baz
 *
 */
static font__attrs splitname(const char *fontname, char *family)
{
  char         tokenised[128];
  char        *tokens[16];
  int          c;
  font__attrs  attrs;
  int          highest_contiguous_unrecognised_token;
  int          i;

  c = tokenise(fontname, tokenised, tokens);

  /* detect attrs */

  attrs = font__WEIGHT_NORMAL  |
          font__STYLE_NORMAL   |
          font__STRETCH_NORMAL |
          font__VARIANT_NORMAL;

  highest_contiguous_unrecognised_token = -1;

  /* examine all the tokens for attrs */

  for (i = 0; i < c; i++)
  {
    const nameattr *m;

    m = bsearch(tokens[i], map, NELEMS(map), sizeof(map[0]), compattrs);
    if (m == NULL)
    {
      DBUG(("unknown token '%s'\n", tokens[i]));

      if (highest_contiguous_unrecognised_token == i - 1)
        highest_contiguous_unrecognised_token = i;
    }
    else
    {
      attrs = (attrs & ~m->mask) | m->value;
    }
  }

  DBUG(("attrs=%x\n", attrs));

  strcpy(family, tokenised);
  for (i = 0; i <= highest_contiguous_unrecognised_token; i++)
  {
    strcat(family, ".");
    strcat(family, tokens[i]);
  }

  DBUG(("font family name is: '%s'\n", family));

  return attrs;
}

error font__select(const char  *wantname,
                   font__attrs  wantattrs,
                   char        *selected,
                   int          selected_size)
{
  char              wantfamily[128];
  font_list_context context;
  int               winner = INT_MAX;

  NOT_USED(selected_size);

  /* Split the wantname up so we have a family name to compare against. We
   * don't care about the attrs. */

  (void) splitname(wantname, wantfamily);

  DBUG(("want %s '%s', %x\n", wantname, wantfamily, wantattrs));

  context = font_RETURN_FONT_NAME;

  for (;;)
  {
    char        fontname[128];
    char        family[128];
    font__attrs attrs;
    int         sc;

    context = font_list_fonts((byte *) fontname,
                              context, sizeof(fontname),
                              NULL, 0,
                              NULL,
                              NULL, NULL);
    if (context == -1)
      break;

    DBUG(("%s\n", fontname));

    attrs = splitname(fontname, family);

    if (strcasecmp(family, wantfamily) != 0)
      continue; /* font family didn't match */

    sc = score(attrs, wantattrs);
    DBUG(("%s scores %d\n", fontname, sc));
    if (sc < winner)
    {
      winner = sc;
      strcpy(selected, fontname);

      if (sc == 0)
      {
        DBUG(("ideal\n"));
        break;
      }
    }
  }

  if (winner == INT_MAX)
    return error_FONT_NO_MATCH;

  return error_OK;
}

font__attrs font__get_attrs(const char *name)
{
  char family[128];

  return splitname(name, family);
}
