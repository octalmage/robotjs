#include "UTHashTable.h"
#include <stdlib.h>
#include <assert.h>

/* Base struct class (all nodes must contain at least the elements in
 * this struct). */
struct _UTHashNode {
	UTHashNode_HEAD
};

typedef struct _UTHashNode UTHashNode;

void initHashTable(UTHashTable *table, size_t initialCount, size_t nodeSize)
{
	assert(table != NULL);
	assert(nodeSize >= sizeof(UTHashNode));

	table->uttable = NULL; /* Must be set to NULL for uthash. */
	table->allocedNodeCount = (initialCount == 0) ? 1 : initialCount;
	table->nodeCount = 0;
	table->nodeSize = nodeSize;
	table->nodes = calloc(table->nodeSize, nodeSize * table->allocedNodeCount);
}

void destroyHashTable(UTHashTable *table)
{
	UTHashNode *uttable = table->uttable;
	UTHashNode *node;

	/* Let uthash do its magic. */
	while (uttable != NULL) {
		node = uttable; /* Grab pointer to first item. */
		HASH_DEL(uttable, node); /* Delete it (table advances to next). */
	}

	/* Only giant malloc'd block containing each node must be freed. */
	if (table->nodes != NULL) free(table->nodes);
	table->uttable = table->nodes = NULL;
}

void *getNewNode(UTHashTable *table)
{
	/* Increment node count, resizing table if necessary. */
	const size_t newNodeCount = ++(table->nodeCount);
	if (table->allocedNodeCount < newNodeCount) {
		do {
			/* Double size each time to avoid calls to realloc(). */
			table->allocedNodeCount <<= 1;
		} while (table->allocedNodeCount < newNodeCount);

		table->nodes = realloc(table->nodes, table->nodeSize *
		                                     table->allocedNodeCount);
	}

	return (char *)table->nodes + (table->nodeSize * (table->nodeCount - 1));
}
