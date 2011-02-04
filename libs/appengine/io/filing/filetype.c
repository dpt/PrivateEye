
#include <string.h>

#include "oslib/types.h"
#include "oslib/osfscontrol.h"

#include "appengine/io/filing.h"

void file_type_to_name(bits file_type, char name[9])
{
  union
  {
    bits bits[2];
    char chars[9];
  }
  filetype;
  char *p;
  char *end;

  osfscontrol_read_file_type(file_type, &filetype.bits[0],
                                        &filetype.bits[1]);

  end = filetype.chars;
  for (p = filetype.chars; p < filetype.chars + 8; )
  {
    int c = *p++;

    if (c > ' ')
      end = p;
    else if (c < ' ')
      break;
    /* otherwise (c == ' ') so keep going */
  }

  memcpy(name, filetype.chars, end - filetype.chars);
  name[end - filetype.chars] = '\0';

#if 0
  c = 0;
  for (i = 0; i < 8; i++)
  {
    int ch = *p++;
    if (ch < ' ')
      break;
    else if (ch > ' ')
      c = i + 1;
    /* else if (ch == ' ') then keep going */
  }

  memcpy(name, filetype.chars, c);
  name[c] = '\0';
#endif
}
