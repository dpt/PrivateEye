/* --------------------------------------------------------------------------
 *    Name: ntree.h
 * Purpose: N-ary tree library
 * ----------------------------------------------------------------------- */

/**
 * \file NTree (interface).
 *
 * NTree is an N-ary tree.
 */

#ifndef APPENGINE_NTREE_H
#define APPENGINE_NTREE_H

#include "appengine/base/errors.h"

#define T ntree_t

typedef struct T T;

/* ----------------------------------------------------------------------- */

error ntree_new(T **t);

/* Unlinks the specified node from the tree. */
void ntree_unlink(T *t);

/* Deletes the specified node and all children. */
void ntree_free(T *t);

/* Unlinks the specified node from the tree, deletes it and all children. */
void ntree_delete(T *t);

/* ----------------------------------------------------------------------- */

T *ntree_nth_child(T *t, int n);

/* ----------------------------------------------------------------------- */

#define ntree_INSERT_AT_END -1

error ntree_prepend(T *parent, T *node);
error ntree_append(T *parent, T *node);
error ntree_insert_before(T *parent, T *sibling, T *node);
error ntree_insert_after(T *parent, T *sibling, T *node);

error ntree_insert(T *t, int where, T *node);

void ntree_set_data(T *t, void *data);
void *ntree_get_data(T *t);

int ntree_depth(T *t);
int ntree_max_height(T *t);
int ntree_n_nodes(T *t);

T *ntree_next_sibling(T *t);
T *ntree_prev_sibling(T *t);
T *ntree_parent(T *t);
T *ntree_first_child(T *t);
T *ntree_last_child(T *t);

/* ----------------------------------------------------------------------- */

typedef unsigned int ntree_walk_flags;

#define ntree_WALK_ORDER_MASK (3u << 0)
#define ntree_WALK_IN_ORDER   (0u << 0)
#define ntree_WALK_PRE_ORDER  (1u << 0)
#define ntree_WALK_POST_ORDER (2u << 0)

#define ntree_WALK_LEAVES     (1u << 2)
#define ntree_WALK_BRANCHES   (1u << 3)
#define ntree_WALK_ALL        (ntree_WALK_LEAVES | ntree_WALK_BRANCHES)

typedef error (ntree_walk_fn)(T *t, void *opaque);

/* max_depth of 0 means 'walk all', 1..N just walk level 1..N */
error ntree_walk(T *t, ntree_walk_flags flags, int max_depth,
                  ntree_walk_fn *fn, void *opaque);

/* ----------------------------------------------------------------------- */

typedef error (ntree_copy_fn)(void *data, void *opaque, void **newdata);

error ntree_copy(T *t, ntree_copy_fn *fn, void *opaque, T **new_t);

#undef T

#endif /* APPENGINE_NTREE_H */
