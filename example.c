#include "region_allocator.h"

typedef struct Node Node;
struct Node {
	Node* left;
	Node* right;
	import_region
};

typedef struct SomeInt SomeInt;
struct SomeInt{
	import_region
};

typedef struct Tree Tree;
struct Tree {
	Region stack;
	Node *root;
};

static void new_tree(Tree *self) {
	new_stack(&self->stack);
	self->root = (Node *)balloc(&self->stack, sizeof(Node) + sizeof(int));
}

static void new_node(Node *self, void *val, size_t size) {
	write_to_region(self->value, val, size);
	self->size = size;
}

static void new_someint(SomeInt *self, int val, size_t size) {
	write_to_region(self->value, &val, size);
	self->size = size;
}

static inline void cleanup_tree(Tree *self) {
	while(self->stack.current_block->back != NULL) {
		self->stack.shrink(&self->stack);
	}

	free(self->stack.current_block);
}

static void insert_node(Node *root, Node *new, int (*cmp)(Node *first, Node *second)) {
	if (cmp(root, new)) {
		root->left = new;
	} else {
		root->right = new;
	}
}

static int cmp_int(Node *first, Node *second) {
	if (*((int *)first->value) >= *((int *)second->value)) {
		return 1;
	} else {
		return 0;
	}
}

static void test_tree(void) {
	Tree tree cleanup(cleanup_tree);
	new_tree(&tree);
	new_node(tree.root, &(int){1000}, sizeof (int));

	Node *node_left = (Node *)balloc(&tree.stack, sizeof(Node) + sizeof(int));
	new_node(node_left, &(int){500}, sizeof (int));
	insert_node(tree.root, node_left, cmp_int);

	Node *node_right = (Node *)balloc(&tree.stack, sizeof(Node) + sizeof(int));
	new_node(node_right, &(int){2000}, sizeof (int));
	insert_node(tree.root, node_right, cmp_int);

	Node *node_left2 = (Node *)balloc(&tree.stack, sizeof(Node) + sizeof(int));
	new_node(node_left2, &(int){250}, sizeof (int));
	insert_node(node_left, node_left2, cmp_int);

	Node *node_right2 = (Node *)balloc(&tree.stack, sizeof(Node) + sizeof(int));
	new_node(node_right2, &(int){4000}, sizeof (int));
	insert_node(node_right, node_right2, cmp_int);

	printf("%d, %zu\n", *(int *)tree.root->left->value, tree.root->left->size);
	printf("%d, %zu\n", *(int *)tree.root->left->left->value, tree.root->left->left->size);
	printf("%d, %zu\n", *(int *)tree.root->right->value, tree.root->right->size);
	printf("%d, %zu\n", *(int *)tree.root->right->right->value, tree.root->right->right->size);
}

int main(void) {
	/* Region stack cleanup(cleanup_stack); */
	Region stack cleanup(cleanup_stack);
	new_stack(&stack);

	test_tree();

	for (int i = 0; i < 100; ++i) {
		SomeInt *new_int_on_stack = (SomeInt *)balloc(&stack, sizeof(SomeInt));
		new_someint(new_int_on_stack, i, sizeof(int));
	}

	int a[1000] = {0};
	bite *p = balloc(&stack, sizeof a);
	write_to_region(p, a, sizeof a);

	int b[1000] = {0};
	p = balloc(&stack, sizeof b);
	write_to_region(p, b, sizeof b);
}
