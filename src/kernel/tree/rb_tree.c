#include <kernel/lock/lock.h>
#include <kernel/tree/rb_tree.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define NIL_NODE (&_rb_tree_nil_node)



static rb_tree_node_t _rb_tree_nil_node={
	.rb_parent_and_color=0,
	.rb_left=NULL,
	.rb_right=NULL
};



static KERNEL_INLINE rb_tree_node_t* _get_parent(rb_tree_node_t* node){
	return (rb_tree_node_t*)(node->rb_parent_and_color&0xfffffffffffffffeull);
}



static KERNEL_INLINE void _set_parent(rb_tree_node_t* node,rb_tree_node_t* parent){
	node->rb_parent_and_color=((u64)parent)|(node->rb_parent_and_color&1);
}



static KERNEL_INLINE _Bool _get_color(rb_tree_node_t* node){
	return node->rb_parent_and_color&1;
}



static KERNEL_INLINE void _set_color(rb_tree_node_t* node,_Bool color){
	node->rb_parent_and_color=(node->rb_parent_and_color&0xfffffffffffffffeull)|color;
}



static KERNEL_INLINE void _replace_node(rb_tree_t* tree,rb_tree_node_t* old,rb_tree_node_t* new){
	rb_tree_node_t* parent=_get_parent(old);
	if (parent){
		parent->rb_nodes[parent->rb_right==old]=new;
	}
	else{
		tree->root=new;
	}
}



void rb_tree_init(rb_tree_t* tree){
	tree->root=NIL_NODE;
	lock_init(&(tree->lock));
}



void rb_tree_insert_node(rb_tree_t* tree,rb_tree_node_t* x){
	panic("Unimplemented: rb_tree_insert_node");
}



void rb_tree_insert_node_increasing(rb_tree_t* tree,rb_tree_node_t* x){
	lock_acquire_exclusive(&(tree->lock));
	x->rb_left=NIL_NODE;
	x->rb_right=NIL_NODE;
	if (tree->root==NIL_NODE){
		x->rb_parent_and_color=0;
		tree->root=x;
		goto _cleanup;
	}
	_set_color(x,1);
	rb_tree_node_t* y=tree->root;
	for (;y->rb_right!=NIL_NODE;y=y->rb_right);
	_set_parent(x,y);
	y->rb_right=x;
	if (!_get_parent(y)){
		goto _cleanup;
	}
	while (1){
		y=_get_parent(x);
		if (!y||!_get_color(y)){
			break;
		}
		_set_color(y,0);
		y=_get_parent(y);
		_set_color(y,1);
		rb_tree_node_t* z=y->rb_left;
		if (_get_color(z)){
			_set_color(z,0);
			x=y;
			continue;
		}
		z=y->rb_right;
		y->rb_right=z->rb_left;
		_set_parent(z->rb_left,y);
		_set_parent(z,_get_parent(y));
		if (_get_parent(y)){
			_get_parent(y)->rb_right=z;
		}
		else{
			tree->root=z;
		}
		z->rb_left=y;
		_set_parent(y,z);
	}
	_set_color(tree->root,0);
_cleanup:
	lock_release_exclusive(&(tree->lock));
}



rb_tree_node_t* rb_tree_lookup_node(rb_tree_t* tree,u64 key){
	lock_acquire_shared(&(tree->lock));
	for (rb_tree_node_t* x=tree->root;x!=NIL_NODE;x=x->rb_nodes[x->key<key]){
		if (x->key==key){
			lock_release_shared(&(tree->lock));
			return x;
		}
	}
	lock_release_shared(&(tree->lock));
	return NULL;
}



void rb_tree_remove_node(rb_tree_t* tree,rb_tree_node_t* x){
	lock_acquire_exclusive(&(tree->lock));
	_Bool skip_recursive_fix=_get_color(x);
	rb_tree_node_t* z;
	rb_tree_node_t* z_parent;
	if (x->rb_left!=NIL_NODE&&x->rb_right!=NIL_NODE){
		rb_tree_node_t* y=x->rb_right;
		for (;y->rb_left!=NIL_NODE;y=y->rb_left);
		skip_recursive_fix=_get_color(y);
		z=y->rb_right;
		z_parent=y;
		if (_get_parent(y)!=x){
			_get_parent(y)->rb_left=y->rb_right;
			y->rb_right=x->rb_right;
			_set_parent(x->rb_right,y);
		}
		_replace_node(tree,x,y);
		y->rb_parent_and_color=x->rb_parent_and_color;
		y->rb_left=x->rb_left;
		_set_parent(y->rb_left,y);
	}
	else{
		z=x->rb_nodes[x->rb_left==NIL_NODE];
		_replace_node(tree,x,z);
		z_parent=_get_parent(x);
		_set_parent(z,z_parent);
	}
	if (skip_recursive_fix){
		goto _cleanup;
	}
	while (z_parent&&!_get_color(z)){
		_Bool i=(z==z_parent->rb_left);
		x=z_parent->rb_nodes[i];
		if (_get_color(x)){
			_set_color(x,0);
			_set_color(z_parent,1);
			rb_tree_node_t* y=x->rb_nodes[!i];
			z_parent->rb_nodes[i]=y;
			_set_parent(y,z_parent);
			_set_parent(x,_get_parent(z_parent));
			_replace_node(tree,z_parent,x);
			x->rb_nodes[!i]=z_parent;
			_set_parent(z_parent,x);
			x=y;
		}
		if (!_get_color(x->rb_left)&&!_get_color(x->rb_right)){
			_set_color(x,1);
			z=z_parent;
			z_parent=_get_parent(z);
			continue;
		}
		if (!_get_color(x->rb_nodes[i])){
			_set_color(x->rb_nodes[!i],0);
			_set_color(x,1);
			rb_tree_node_t* y=x->rb_nodes[!i];
			x->rb_nodes[!i]=y->rb_nodes[i];
			_set_parent(y->rb_nodes[i],x);
			_set_parent(y,_get_parent(x));
			_get_parent(x)->rb_nodes[x==_get_parent(x)->rb_right]=y;
			y->rb_nodes[i]=x;
			_set_parent(x,y);
			x=z_parent->rb_nodes[i];
		}
		_set_color(x,_get_color(z_parent));
		_set_color(z_parent,0);
		_set_color(x->rb_nodes[i],0);
		z_parent->rb_nodes[i]=x->rb_nodes[!i];
		_set_parent(x->rb_nodes[!i],z_parent);
		_set_parent(x,_get_parent(z_parent));
		_replace_node(tree,z_parent,x);
		x->rb_nodes[!i]=z_parent;
		_set_parent(z_parent,x);
		_set_color(tree->root,0);
		goto _cleanup;
	}
	_set_color(z,0);
_cleanup:
	lock_release_exclusive(&(tree->lock));
}



rb_tree_node_t* rb_tree_iter_start(rb_tree_t* tree){
	if (tree->root==NIL_NODE){
		return NULL;
	}
	lock_acquire_shared(&(tree->lock));
	rb_tree_node_t* x=tree->root;
	for (;x->rb_left!=NIL_NODE;x=x->rb_left);
	lock_release_shared(&(tree->lock));
	return x;
}



rb_tree_node_t* rb_tree_iter_next(rb_tree_t* tree,rb_tree_node_t* x){
	lock_acquire_shared(&(tree->lock));
	if (x->rb_right!=NIL_NODE){
		for (x=x->rb_right;x->rb_left!=NIL_NODE;x=x->rb_left);
	}
	else{
		rb_tree_node_t* y;
		do{
			y=x;
			x=_get_parent(x);
		} while (x&&y==x->rb_right);
	}
	lock_release_shared(&(tree->lock));
	return x;
}