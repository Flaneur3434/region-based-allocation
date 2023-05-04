When writing to memory in region, you have to use write_to_region

Obviously you need to push first ...

Example:

Suppose we want to push the 'Node' struct to the region.  All metadata is at the
top of the struct.  Data is at the bottom, using a variadic array. This way,
we dont need many pointer re-directions

```c
struct Node
	Node* left;
	Node* right;
	import_region
};

static void new_node(Node *self, void *val, size_t size)
	write_to_region(self->value, val, size);
	self->size = size;
}
```