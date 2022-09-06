// license: mit
/* from: https://github.com/robin-thomas/min-heap/blob/a1a8d7137f3afdf2b5ebf93b9d4059c4d1dd96e8/minHeap.c */

#pragma once

#include "util.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef HEAP_COMPARE
#define HEAP_COMPARE(meta, x, y) compare(meta, x, y) > 0
#endif

#define HEAP_LCHILD(x) 2 * x + 1
#define HEAP_RCHILD(x) 2 * x + 2
#define HEAP_PARENT(x) (x - 1) / 2

typedef struct heap_s {
	u16 meta;
    i32 size;
    u8 **nodes;
} heap_t;

void heap_swap(u8 **n1, u8 **n2) {
    u8* temp = *n1;
    *n1 = *n2;
    *n2 = temp;
}

void heap_heapify(heap_t *h, i32 i) {
    i32 smallest = (HEAP_LCHILD(i) < h->size && HEAP_COMPARE(h->meta, h->nodes[HEAP_LCHILD(i)], h->nodes[i])) ? HEAP_LCHILD(i) : i;
    if(HEAP_RCHILD(i) < h->size && HEAP_COMPARE(h->meta, h->nodes[HEAP_RCHILD(i)], h->nodes[smallest]))
        smallest = HEAP_RCHILD(i);
    if(smallest != i) {
		heap_swap(&(h->nodes[i]), &(h->nodes[smallest]));
        heap_heapify(h, smallest);
    }
}

void heap_insert(heap_t *h, u8 *data) {
    if(h->size)
        h->nodes = realloc(h->nodes, (h->size + 1) * sizeof(u8*));
    else
        h->nodes = malloc(sizeof(u8*));
    i32 i = (h->size)++;
    while(i && HEAP_COMPARE(h->meta, data, h->nodes[HEAP_PARENT(i)])) {
        h->nodes[i] = h->nodes[HEAP_PARENT(i)];
        i = HEAP_PARENT(i);
    }
    h->nodes[i] = data;
}

void heap_delete(heap_t *h) {
    if(h->size) {
        h->nodes[0] = h->nodes[--(h->size)];
        h->nodes = realloc(h->nodes, h->size * sizeof(u8*));
        heap_heapify(h, 0);
    }
}

void heapify(heap_t *h, i32 i) {
    i32 smallest = (HEAP_LCHILD(i) < h->size && h->nodes[HEAP_LCHILD(i)] < h->nodes[i]) ? HEAP_LCHILD(i) : i;
    if(HEAP_RCHILD(i) < h->size && HEAP_COMPARE(h->meta, h->nodes[HEAP_RCHILD(i)], h->nodes[smallest])) {
        smallest = HEAP_RCHILD(i);
    }
    if(smallest != i) {
        heap_swap(&(h->nodes[i]), &(h->nodes[smallest]));
        heapify(h, smallest);
    }
}

void heap_truncate(heap_t *h, i32 size) {
	if (h->size <= size)
		return;
	heap_t h2 = {0};
	h2.meta = h->meta;
	for (i32 i = 0; i < size; i++) {
		heap_insert(&h2, h->nodes[0]);
		heap_delete(h);
	}
	while (h->size) {
		free(h->nodes[0]);
		heap_delete(h);
	}
	h->nodes = h2.nodes;
	h->size = size;
}

void heap_free(heap_t *h) {
    free(h->nodes);
}
