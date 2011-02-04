/* --------------------------------------------------------------------------
 *    Name: set-handlers.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

void tag_cloud__set_handlers(tag_cloud              *tc,
                             tag_cloud__newtagfn    *newtag,
                             tag_cloud__deletetagfn *deletetag,
                             tag_cloud__renametagfn *renametag,
                             tag_cloud__tagfn       *tag,
                             tag_cloud__tagfn       *detag,
                             tag_cloud__tagfilefn   *tagfile,
                             tag_cloud__tagfilefn   *detagfile,
                             tag_cloud__eventfn     *event,
                             void                   *arg)
{
  tc->newtag    = newtag;
  tc->deletetag = deletetag;
  tc->renametag = renametag;
  tc->tag       = tag;
  tc->detag     = detag;
  tc->arg       = arg;
  tc->tagfile   = tagfile;
  tc->detagfile = detagfile;
  tc->event     = event;
}

void tag_cloud__set_key_handler(tag_cloud                 *tc,
                                tag_cloud__key_handler_fn *handler,
                                void                      *arg)
{
  tc->key_handler     = handler;
  tc->key_handler_arg = arg;
}
