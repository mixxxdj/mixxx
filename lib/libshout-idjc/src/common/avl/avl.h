/*
 * Copyright (C) 1995 by Sam Rushing <rushing@nightmare.com>
 */

/* $Id: avl.h,v 1.7 2003/07/07 01:10:14 brendan Exp $ */

#ifndef __AVL_H
#define __AVL_H

#ifdef __cplusplus
extern "C" {
#endif

#define AVL_KEY_PRINTER_BUFLEN (256)

#ifndef NO_THREAD
#include "thread/thread.h"
#else
#define thread_rwlock_create(x) do{}while(0)
#define thread_rwlock_destroy(x) do{}while(0)
#define thread_rwlock_rlock(x) do{}while(0)
#define thread_rwlock_wlock(x) do{}while(0)
#define thread_rwlock_unlock(x) do{}while(0)
#endif

typedef struct avl_node_tag {
  void *        key;
  struct avl_node_tag *    left;
  struct avl_node_tag *    right;  
  struct avl_node_tag *    parent;
  /*
   * The lower 2 bits of <rank_and_balance> specify the balance
   * factor: 00==-1, 01==0, 10==+1.
   * The rest of the bits are used for <rank>
   */
  unsigned int        rank_and_balance;
#if !defined(NO_THREAD) && defined(HAVE_AVL_NODE_LOCK)
  rwlock_t rwlock;
#endif
} avl_node;

#define AVL_GET_BALANCE(n)    ((int)(((n)->rank_and_balance & 3) - 1))

#define AVL_GET_RANK(n)    (((n)->rank_and_balance >> 2))

#define AVL_SET_BALANCE(n,b) \
  ((n)->rank_and_balance) = \
    (((n)->rank_and_balance & (~3)) | ((int)((b) + 1)))

#define AVL_SET_RANK(n,r) \
  ((n)->rank_and_balance) = \
    (((n)->rank_and_balance & 3) | (r << 2))

struct _avl_tree;

typedef int (*avl_key_compare_fun_type)    (void * compare_arg, void * a, void * b);
typedef int (*avl_iter_fun_type)    (void * key, void * iter_arg);
typedef int (*avl_iter_index_fun_type)    (unsigned long index, void * key, void * iter_arg);
typedef int (*avl_free_key_fun_type)    (void * key);
typedef int (*avl_key_printer_fun_type)    (char *, void *);

/*
 * <compare_fun> and <compare_arg> let us associate a particular compare
 * function with each tree, separately.
 */

#ifdef _mangle
# define avl_tree_new _mangle(avl_tree_new)
# define avl_node_new _mangle(avl_node_new)
# define avl_tree_free _mangle(avl_tree_free)
# define avl_insert _mangle(avl_insert)
# define avl_delete _mangle(avl_delete)
# define avl_get_by_index _mangle(avl_get_by_index)
# define avl_get_by_key _mangle(avl_get_by_key)
# define avl_iterate_inorder _mangle(avl_iterate_inorder)
# define avl_iterate_index_range _mangle(avl_iterate_index_range)
# define avl_tree_rlock _mangle(avl_tree_rlock)
# define avl_tree_wlock _mangle(avl_tree_wlock)
# define avl_tree_wlock _mangle(avl_tree_wlock)
# define avl_tree_unlock _mangle(avl_tree_unlock)
# define avl_node_rlock _mangle(avl_node_rlock)
# define avl_node_wlock _mangle(avl_node_wlock)
# define avl_node_unlock _mangle(avl_node_unlock)
# define avl_get_span_by_key _mangle(avl_get_span_by_key)
# define avl_get_span_by_two_keys _mangle(avl_get_span_by_two_keys)
# define avl_verify _mangle(avl_verify)
# define avl_print_tree _mangle(avl_print_tree)
# define avl_get_first _mangle(avl_get_first)
# define avl_get_prev _mangle(avl_get_prev)
# define avl_get_next _mangle(avl_get_next)
# define avl_get_item_by_key_most _mangle(avl_get_item_by_key_most)
# define avl_get_item_by_key_least _mangle(avl_get_item_by_key_least)
#endif

typedef struct _avl_tree {
  avl_node *            root;
  unsigned int          height;
  unsigned int          length;
  avl_key_compare_fun_type    compare_fun;
  void *             compare_arg;
#ifndef NO_THREAD
  rwlock_t rwlock;
#endif
} avl_tree;

avl_tree * avl_tree_new (avl_key_compare_fun_type compare_fun, void * compare_arg);
avl_node * avl_node_new (void * key, avl_node * parent);

void avl_tree_free (
  avl_tree *        tree,
  avl_free_key_fun_type    free_key_fun
  );

int avl_insert (
  avl_tree *        ob,
  void *        key
  );

int avl_delete (
  avl_tree *        tree,
  void *        key,
  avl_free_key_fun_type    free_key_fun
  );

int avl_get_by_index (
  avl_tree *        tree,
  unsigned long        index,
  void **        value_address
  );

int avl_get_by_key (
  avl_tree *        tree,
  void *        key,
  void **        value_address
  );

int avl_iterate_inorder (
  avl_tree *        tree,
  avl_iter_fun_type    iter_fun,
  void *        iter_arg
  );

int avl_iterate_index_range (
  avl_tree *        tree,
  avl_iter_index_fun_type iter_fun,
  unsigned long        low,
  unsigned long        high,
  void *        iter_arg
  );

int avl_get_span_by_key (
  avl_tree *        tree,
  void *        key,
  unsigned long *    low,
  unsigned long *    high
  );

int avl_get_span_by_two_keys (
  avl_tree *        tree,
  void *        key_a,
  void *        key_b,
  unsigned long *    low,
  unsigned long *    high
  );

int avl_verify (avl_tree * tree);

void avl_print_tree (
  avl_tree *        tree,
  avl_key_printer_fun_type key_printer
  );

avl_node *avl_get_first(avl_tree *tree);

avl_node *avl_get_prev(avl_node * node);

avl_node *avl_get_next(avl_node * node);

/* These two are from David Ascher <david_ascher@brown.edu> */

int avl_get_item_by_key_most (
  avl_tree *        tree,
  void *        key,
  void **        value_address
  );

int avl_get_item_by_key_least (
  avl_tree *        tree,
  void *        key,
  void **        value_address
  );

/* optional locking stuff */
void avl_tree_rlock(avl_tree *tree);
void avl_tree_wlock(avl_tree *tree);
void avl_tree_unlock(avl_tree *tree);
void avl_node_rlock(avl_node *node);
void avl_node_wlock(avl_node *node);
void avl_node_unlock(avl_node *node);

#ifdef __cplusplus
}
#endif

#endif /* __AVL_H */
