/* --------------------------------------------------------------------------
 *    Name: ntree.h
 * Purpose: N-ary tree
 * ----------------------------------------------------------------------- */

#ifndef APPENGINE_NTREE_H
#define APPENGINE_NTREE_H

#include "appengine/base/errors.h"

#define T ntree_t

typedef struct T T;

/* ----------------------------------------------------------------------- */

error ntree__new(T **t);

/* Unlinks the specified node from the tree. */
void ntree__unlink(T *t);

/* Deletes the specified node and all children. */
void ntree__free(T *t);

/* Unlinks the specified node from the tree, deletes it and all children. */
void ntree__delete(T *t);

/* ----------------------------------------------------------------------- */

T *ntree__nth_child(T *t, int n);

/* ----------------------------------------------------------------------- */

#define ntree__INSERT_AT_END -1

error ntree__prepend(T *parent, T *node);
error ntree__append(T *parent, T *node);
error ntree__insert_before(T *parent, T *sibling, T *node);
error ntree__insert_after(T *parent, T *sibling, T *node);

error ntree__insert(T *t, int where, T *node);

void ntree__set_data(T *t, void *data);
void *ntree__get_data(T *t);

int ntree__depth(T *t);
int ntree__max_height(T *t);
int ntree__n_nodes(T *t);

T *ntree__next_sibling(T *t);
T *ntree__prev_sibling(T *t);
T *ntree__parent(T *t);
T *ntree__first_child(T *t);
T *ntree__last_child(T *t);

/* ----------------------------------------------------------------------- */

typedef unsigned int ntree__walk_flags;

#define ntree__WALK_ORDER_MASK (3u << 0)
#define ntree__WALK_IN_ORDER   (0u << 0)
#define ntree__WALK_PRE_ORDER  (1u << 0)
#define ntree__WALK_POST_ORDER (2u << 0)

#define ntree__WALK_LEAVES     (1u << 2)
#define ntree__WALK_BRANCHES   (1u << 3)
#define ntree__WALK_ALL        (ntree__WALK_LEAVES | ntree__WALK_BRANCHES)

typedef error (ntree__walk_fn)(T *t, void *arg);

/* max_depth of 0 means 'walk all', 1..N just walk level 1..N */
error ntree__walk(T *t, ntree__walk_flags flags, int max_depth,
                  ntree__walk_fn *fn, void *arg);

/* ----------------------------------------------------------------------- */

typedef error (ntree__copy_fn)(void *data, void *arg, void **newdata);

error ntree__copy(T *t, ntree__copy_fn *fn, void *arg, T **new_t);

#undef T

#endif /* APPENGINE_NTREE_H */
