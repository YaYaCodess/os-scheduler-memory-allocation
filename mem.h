#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MEM_SZ(x) memory_cell_size_to_level( x )

typedef struct memory_cell {
	int size;
	int level; // 0 - 10 (0 for 1024)
	int taken;

	int index;

	struct memory_cell** children;
} memory_cell;

int memory_cell_size_to_level(int size) {
	if (size <= 0 || size > 1024) return -1;

	return 10 - (int)ceilf(log2f(size));
}

memory_cell* memory_cell_create(int size, int* globalIndex) {
	if (size <= 0) return 0;

	struct memory_cell* cell = (struct memory_cell*)malloc(sizeof(memory_cell));
	memset(cell, 0, sizeof(memory_cell));

	cell->size = size;
	cell->level = memory_cell_size_to_level(size);

	cell->index = -1;

	if (size > 1) {
		cell->children = (struct memory_cell**)malloc(sizeof(memory_cell*) * 2);

		for (int i = 0; i < 2; i++) {
			cell->children[i] = memory_cell_create(size / 2, globalIndex);
		}
	}
	else {
		// set index
		cell->index = (*globalIndex)++;
	}

	return cell;
}


int memory_internal_any_taken(memory_cell* mem) {
	if (!mem) return 0;

	if (mem->taken) return 1;

	if (mem->size == 1) return mem->taken;

	return memory_internal_any_taken(mem->children[0]) || memory_internal_any_taken(mem->children[1]);
}

// can abdo steal this memory?
int memory_cell_allocate(memory_cell* mem, int level, memory_cell** allocatedCell) {
	if (!mem || mem->taken || *allocatedCell || level > 10 || level < 0) return 0;

	if (mem->level == level) {
		if (memory_internal_any_taken(mem)) {
			return 0;
		}

		mem->taken = 1;
		
		*allocatedCell = mem;
		return 1;
	}

	return memory_cell_allocate(mem->children[0], level, allocatedCell) || memory_cell_allocate(mem->children[1], level, allocatedCell);
}

int memory_cell_free(memory_cell* mem) {
	if (!mem || !mem->taken) return 0;

	mem->taken = 0;
	return 1;
}

int memory_internal_child_index(memory_cell* mem, int childIdx) {
	if (!mem) return -1;

	if (mem->size == 1) return mem->index;

	return memory_internal_child_index(mem->children[childIdx], childIdx);
}


void memory_cell_get_indices(memory_cell* mem, int* start, int* end) {
	if (!mem) return;

	if (start) {
		*start = memory_internal_child_index(mem, 0);
	}

	if (end) {
		*end = memory_internal_child_index(mem, 1);
	}
}