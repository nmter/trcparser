#include "rb_tree.h"



rb_tree_t rbtree = {
	0,
	NULL,
	&rb_alloc_new,
	&rb_insert,
	NULL,
	&rb_destroy,
	&rb_print,

	&in_tree_callback
};

int _rb_compare(_key_t key1, _key_t key2)// UINT32* [4]
{
	int i = 3;
	while(i >= 0){
		if(key1[i] > key2[i]){
			return 1;
		}else if(key1[i] < key2[i]){
			return -1;
		}
		i--;
	}
	return 0;
}


rb_node_t* _rb_search(_key_t key)// 
{
	rb_node_t *ptr = rbtree.root, *parent = NULL;
	int ret;
	while(ptr){
		if((ret = _rb_compare(key, ptr->key)) == 0){
			return ptr;
		}else if(ret > 0){
			parent = ptr;
			ptr = ptr->rb_right;
		}else{
			parent = ptr;
			ptr = ptr->rb_left;
		}
	}
	return parent;//if NULL, means the rbtree is empty.
}

rb_node_t* rb_alloc_new(_key_t key)
{
	rb_node_t *temp = malloc(sizeof(rb_node_t));
	memset(temp, 0, sizeof(rb_node_t));
	temp->key = key;
	return temp;
}

void _rb_rotate_right(rb_node_t *org_subroot)
{
	rb_node_t *y = org_subroot->rb_left;
	//y is not NULL
	org_subroot->rb_left = y->rb_right;
	if(org_subroot->rb_left){
		org_subroot->rb_left->rb_parent = org_subroot;
	}
	y->rb_parent = org_subroot->rb_parent;
	if(org_subroot->rb_parent){
		if(org_subroot == org_subroot->rb_parent->rb_left){
			org_subroot->rb_parent->rb_left = y;
		}else{
			org_subroot->rb_parent->rb_right = y;
		}
	}else{
		rbtree.root = y;
	}
	y->rb_right = org_subroot;
	org_subroot->rb_parent = y;
}

void _rb_rotate_left(rb_node_t *org_subroot)
{
	rb_node_t *y = org_subroot->rb_right;
	org_subroot->rb_right = y->rb_left;
	if(org_subroot->rb_right){
		org_subroot->rb_right->rb_parent = org_subroot;
	}
	y->rb_parent = org_subroot->rb_parent;
	if(org_subroot->rb_parent){
		if(org_subroot == org_subroot->rb_parent->rb_left){
			org_subroot->rb_parent->rb_left = y;
		}else{
			org_subroot->rb_parent->rb_right = y;
		}
	}else{
		rbtree.root = y;
	}
	y->rb_left = org_subroot;
	org_subroot->rb_parent = y;
}

void _rb_insert_fix(rb_node_t *node)
{
	rb_node_t *parent = node->rb_parent, *uncle;
	while(parent && (!parent->color)){
		//parent.color = 0 red
		if(parent == parent->rb_parent->rb_left){
			uncle = parent->rb_parent->rb_right;
			if(uncle && (!uncle->color)){
				parent->color = uncle->color = 1;
				parent->rb_parent->color = 0;
				node = parent->rb_parent;
				parent = node->rb_parent;
				continue;
			}else{
				//uncle is black 1
				if(node == parent->rb_left){
					parent->color = 1;
					parent->rb_parent->color = 0;
					_rb_rotate_right(parent->rb_parent);
					break;
				}else{
					_rb_rotate_left(parent);
					node = parent;
					parent = node->rb_parent;
					continue;
				}
			}
		}else{
			uncle = parent->rb_parent->rb_left;
			if(uncle && (!uncle->color)){
				parent->color = uncle->color = 1;
				parent->rb_parent->color = 0;
				node = parent->rb_parent;
				parent = node->rb_parent;
				continue;
			}else{
				if(node == parent->rb_right){
					parent->color = 1;
					parent->rb_parent->color = 0;
					_rb_rotate_left(parent->rb_parent);
					break;
				}else{
					_rb_rotate_right(parent);
					node = parent;
					parent = node->rb_parent;
					continue;
				}
			}
		}
	}
}

void _rb_travel_rbt(int mode, rb_node_t *ptr)//recursive travel rbtree
{
	if(ptr){
		switch(mode){
			case 0:
				rbtree.what_to_do(ptr);
				_rb_travel_rbt(0, ptr->rb_left);
				_rb_travel_rbt(0, ptr->rb_right);
				break;
			case 1:
				_rb_travel_rbt(1, ptr->rb_left);
				rbtree.what_to_do(ptr);
				_rb_travel_rbt(1, ptr->rb_right);
				break;
			case 2:
				_rb_travel_rbt(2, ptr->rb_left);
				_rb_travel_rbt(2, ptr->rb_right);
				rbtree.what_to_do(ptr);
				break;
		}
	}
}



int rb_insert(rb_node_t *node)//must alloc before
{
	rb_node_t *ptr = _rb_search(node->key);
	int ret;
	if(ptr){
		if((ret = _rb_compare(node->key, ptr->key)) == 0){
			in_tree_callback();
			return -1;
		}else if(ret > 0){
			ptr->rb_right = node;//node->color == 0 red
		}else{
			ptr->rb_left = node;
		}
		node->rb_parent = ptr;
		_rb_insert_fix(node);
	}else{
		rbtree.root = node;
	}
	rbtree.root->color = 1;
	rbtree.num++;
	return 0;
}

void _rb_print_node(rb_node_t *node)
{
	printf("%d\n",*(node->key));
}

void _rb_freekey(_key_t key){
	
	//printf("key: %d free!\n",*key);
	free(key);
}

void _rb_destroy_node(rb_node_t *node)
{
	_rb_freekey(node->key);
	free(node);
}

void rb_print()
{
	rbtree.what_to_do = &_rb_print_node;
	_rb_travel_rbt(0, rbtree.root);
}

void rb_destroy()
{
	rbtree.what_to_do = &_rb_destroy_node;
	_rb_travel_rbt(2, rbtree.root);
}