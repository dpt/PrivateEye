
#include <stdio.h>

#include "fortify/fortify.h"

#include "appengine/types.h"

typedef int (testfn)(void);

extern int array_test(void);
extern int atom_test(void);
extern int bitarr_test(void);
extern int bitvec_test(void);
extern int hash_test(void);
extern int list_test(void);
extern int ntree_test(void);
extern int vector_test(void);
extern int tagdb_test(void);
extern int txtfmt_test(void);
extern int filing_test(void);
extern int font_test(void);
extern int lang_test(void);
extern int packer_test(void);
extern int layout_test(void);
extern int stream_test(void);
extern int pack_test(void);

typedef struct test
{
  const char *name;
  testfn     *test;
}
test;

static const test tests[] =
{
  { "array",  array_test  },
  { "atom",   atom_test   },
  { "bitarr", bitarr_test },
  { "bitvec", bitvec_test },
  { "hash",   hash_test   },
  { "list",   list_test   },
  { "ntree",  ntree_test  },
  { "vector", vector_test },
  { "tagdb",  tagdb_test  },
  { "txtfmt", txtfmt_test },
  { "filing", filing_test },
  { "font",   font_test   },
  { "lang",   lang_test   },
  { "packer", packer_test },
  { "layout", layout_test },
  { "stream", stream_test },
  { "pack",   pack_test   },
};

static const int ntests = NELEMS(tests);

static int runtest(const test *t)
{
  int rc;

  printf(">> %s tests\n", t->name);

  rc = t->test();

  printf("\n");

  Fortify_CheckAllMemory();

  return rc;
}

int main(int argc, char *argv[])
{
  int nrun;
  int nfailures;
  int i;
  int passed;

  Fortify_EnterScope();

  printf("Starting tests.\n");

  nrun      = 0;
  nfailures = 0;

  if (argc < 2)
  {
    /* run all tests */

    for (i = 0; i < ntests; i++)
    {
      nrun++;
      if (runtest(&tests[i]))
      {
        nfailures++;
        printf("********************\n");
        printf("*** TEST FAILING ***\n");
        printf("********************\n");
        printf("\n");
      }
    }
  }
  else
  {
    /* run the specified test */

    for (i = 1; i < argc; i++)
    {
      int j;

      for (j = 0; j < ntests; j++)
        if (strcmp(tests[j].name, argv[i]) == 0)
          break;

      if (j == ntests)
      {
        printf("Unknown test '%s'.\n", argv[i]);
      }
      else
      {
        nrun++;
        if (runtest(&tests[j]))
        {
          nfailures++;
          printf("********************\n");
          printf("*** TEST FAILING ***\n");
          printf("********************\n");
          printf("\n");
        }
      }
    }
  }

  passed = nrun - nfailures;

  printf("\nTests completed: %d/%d tests passed.\n", passed, nrun);

  Fortify_LeaveScope();
  Fortify_OutputStatistics();
}
