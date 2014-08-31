#pragma once
#ifndef UTHASHTABLE_H
#define UTHASHTABLE_H

#include <stddef.h>
#include "uthash.h"

/* All node structs must begin with this (note that there is NO semicolon). */
#define UTHashNode_HEAD UT_hash_handle hh;

/* This file contains convenience macros and a standard struct for working with
 * uthash hash tables.
 *
 * The main purpose of this is for convenience of creating/freeing nodes. */
struct _UTHashTable {
	void *uttable; /* The uthash table -- must start out as NULL. */
	void *nodes; /* Contiguous array of nodes. */
	size_t allocedNodeCount; /* Node count currently allocated for. */
	size_t nodeCount; /* Current node count. */
	size_t nodeSize; /* Size of each node. */
};

typedef struct _UTHashTable UTHashTable;

/* Initiates a hash table to the default values. |table| should point to an
 * already allocated UTHashTable struct.
 *
 * If the |initialCount| argument in initHashTable is given, |nodes| is
 * allocated immediately to the maximum size and new nodes are simply slices of
 * that array. This can save calls to malloc if many nodes are to be added, and
 * the a reasonable maximum number is known ahead of time.
 *
 * If the node count goes over this maximum, or if |initialCount| is 0, the
 * array is dynamically reallocated to fit the size.
 */
void initHashTable(UTHashTable *table, size_t initialCount, size_t nodeSize);

/* Frees memory occupied by a UTHashTable's members.
 *
 * Note that this does NOT free memory for the UTHashTable pointed to by
 * |table| itself; if that was allocated on the heap, you must free() it
 * yourself after calling this. */
void destroyHashTable(UTHashTable *table);

/* Returns memory allocated for a new node. Responsibility for freeing this is
 * up to the destroyHashTable() macro; this should NOT be freed by the caller.
 *
 * This is intended to be used with a HASH_ADD() macro, e.g.:
 * {%
 *     struct myNode *uttable = utHashTable->uttable;
 *     struct myNode *node = getNewNode(utHashTable);
 *     node->key = 42;
 *     node->value = someValue;
 *     HASH_ADD_INT(uttable, key, node);
 *     utHashTable->uttable = uttable;
 * %}
 *
 * Or, use the UTHASHTABLE_ADD_INT or UTHASHTABLE_ADD_STR macros
 * for convenience (they are exactly equivalent):
 * {%
 *     struct myNode *node = getNewNode(utHashTable);
 *     node->key = 42;
 *     node->value = someValue;
 *     UTHASHTABLE_ADD_INT(utHashTable, key, node, struct myNode);
 * %}
 */
void *getNewNode(UTHashTable *table);

#define UTHASHTABLE_ADD_INT(tablePtr, keyName, node, nodeType) \
do {                                       \
  nodeType *uttable = (tablePtr)->uttable; \
  HASH_ADD_INT(uttable, keyName, node);    \
  (tablePtr)->uttable = uttable;           \
} while (0)

#define UTHASHTABLE_ADD_STR(tablePtr, keyName, node, nodeType) \
do {                                       \
  nodeType *uttable = (tablePtr)->uttable; \
  HASH_ADD_STR(uttable, keyName, node);    \
  (tablePtr)->uttable = uttable;           \
} while (0)

#endif /* MMHASHTABLE_H */
