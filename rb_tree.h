#ifndef _RB_TREE_H
#define _RB_TREE_H
#include "trc_parser.h"
#include <string.h>
#include <malloc.h>

typedef UINT32* _key_t;

extern void in_tree_callback();


typedef struct _rb_node_t{
	_key_t key; //"key" - its type remain to change to template-c++ [int array]
	struct _rb_node_t *rb_left, *rb_right, *rb_parent;
	char color;// 0 - red, 1 - black
}rb_node_t;
typedef struct _rb_tree_t{
	int num;
	rb_node_t *root;
	/*some functions*/
	rb_node_t* ( *rb_alloc_new)(_key_t key);
	
	int ( *rb_insert)(rb_node_t *node);

	void (*what_to_do)(rb_node_t* node);
	void ( *rb_destroy)();
	void ( *rb_print)();

	void ( *in_tree_callback)();
	//rb_remove: don't need it right now
}rb_tree_t;

rb_node_t* rb_alloc_new(_key_t key);
int rb_insert(rb_node_t *node);
void rb_destroy();
void rb_print();

#endif