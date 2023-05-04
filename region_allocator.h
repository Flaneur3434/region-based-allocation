#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define cleanup(x) __attribute__((cleanup(x)))
#define overloadable __attribute__((overloadable))

#define BLOCK_SIZE (32)
#define BLOCK_PADDING (4)

typedef unsigned char bite;

typedef struct Block Block;
struct Block {
	bite *sp;             /* stack pointer */
	Block *next;          /* Next memory block */
	Block *back;          /* Memory block behind */
	bite memory[];        /* Actual writable memory region */
};

typedef struct Region Region;
struct Region {
	Block *stack;                                         /* Head pointer (sentinel) of doublely linked list */
	Block *current_block;                                 /* Pointer to current memory block being filled */
	size_t block_size;                                    /* The size of writable memory for each block */
	void (*grow)(Region *self, size_t size);              /* Function that adds blocks to linked list if memory runs out */
	void (*shrink)(Region *self);                         /* Function that frees blocks when done using */
	bite *(*region_alloc)(Region *self, size_t size);     /* Function that moves stack pointer for current memory block */
};

/* Constructor for block in linked list */
Block *New_block(size_t size) {
	Block *new_block = (Block *)malloc(sizeof(Block) + size);
	new_block->sp = new_block->memory;
	new_block->next = NULL;
	new_block->back = NULL;

	return new_block;
}

void Grow(Region *self, size_t size) {
	Block *temp = self->current_block;
	if (size > self->block_size) {
		/* A new bigger size is allocated */
		self->block_size = size + BLOCK_PADDING;
	} else {
		/* If same size is allocated again */
		self->block_size = (self->block_size - BLOCK_PADDING) * 2 + BLOCK_PADDING;
	}

	self->current_block = New_block(self->block_size);
	temp->next = self->current_block;
	self->current_block->back = temp;
}

void Shrink(Region *self) {
	Block *temp = self->current_block;
	self->current_block = self->current_block->back;
	free(temp);
}

bite *Block_alloc(Region *self, size_t size) {
	if (size % 8 != 0) {
		size += (size % 8);
	}

	if (self->block_size - (self->current_block->sp - self->current_block->memory) < size) {
		self->grow(self, size);
	}

	bite *temp = self->current_block->sp;
	self->current_block->sp += (bite) size;
	return temp;
}

void cleanup_stack(Region *self) {
	while(self->current_block->back != NULL) {
		self->shrink(self);
	}

	free(self->current_block);
}

/* Constructor for user facing arena struct */
void new_stack (Region *self) {
	self->block_size = BLOCK_SIZE + BLOCK_PADDING;
	self->stack = New_block(self->block_size);
	self->current_block = self->stack;
	self->grow = Grow;
	self->shrink = Shrink;
	self->region_alloc = Block_alloc;
}

/*
 * User facing allocation function
 * Basically malloc
 */
bite *balloc(Region *self, size_t size) {
	return self->region_alloc(self, size);
}

/*
 * User facing memcpy function to hide details
 * dest pointer is the pointer on stack
 * source is whatever data the user wants to write to stack
 * size is the size of source
 * Basically memcpy
 */
void write_to_region(void *dest, void *source, size_t size) {
	memcpy(dest, source, size);
}

/*
 * User should add this macro at the end of structs they intend to use with
 * stack_mem
 */
#define import_region				\
size_t size;					\
bite value[];
