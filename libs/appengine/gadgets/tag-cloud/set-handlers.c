/* --------------------------------------------------------------------------
 *    Name: set-handlers.c
 * Purpose: Tag cloud
 * ----------------------------------------------------------------------- */

#include "appengine/gadgets/tag-cloud.h"

#include "impl.h"

void tag_cloud_set_handlers(tag_cloud              *tc,
                             tag_cloud_newtagfn    *newtag,
                             tag_cloud_deletetagfn *deletetag,
                             tag_cloud_renametagfn *renametag,
                             tag_cloud_tagfn       *tag,
                             tag_cloud_tagfn       *detag,
                             tag_cloud_tagfilefn   *tagfile,
                             tag_cloud_tagfilefn   *detagfile,
                             tag_cloud_eventfn     *event,
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

void tag_cloud_set_key_handler(tag_cloud                 *tc,
                                tag_cloud_key_handler_fn *handler,
                                void                      *arg)
{
  tc->key_handler     = handler;
  tc->key_handler_arg = arg;
}
