#include <glsl/_internal/error.h>
#include <glsl/_internal/operator_table.h>
#include <glsl/ast.h>
#include <glsl/builtin_types.h>
#include <glsl/lexer.h>
#include <sys/types.h>



#define BUILD_NODE(node) GLSL_AST_NODE_TYPE_##node
#define BUILD_TYPE(type) GLSL_BUILTIN_TYPE_##type

#define MATCH(left_type,right_type) (glsl_builtin_type_is_compatible(left->value_type->builtin_type,BUILD_TYPE(left_type))&&glsl_builtin_type_is_compatible(right->value_type->builtin_type,BUILD_TYPE(right_type)))
#define MATCH_EXACT(left_type,right_type) (left->value_type->builtin_type==BUILD_TYPE(left_type)&&right->value_type->builtin_type==BUILD_TYPE(right_type))

#define MATCH_AND_RETURN(node_type,left_type,right_type,value_type) \
	if (MATCH(left_type,right_type)){ \
		return _build_return_type(left,right,BUILD_NODE(node_type),BUILD_TYPE(value_type)); \
	}



static glsl_ast_node_t* _build_return_type(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_ast_node_type_t node_type,glsl_builtin_type_t value_type){
	glsl_ast_node_t* out=glsl_ast_node_create(node_type);
	out->value_type=glsl_ast_type_create(GLSL_AST_TYPE_TYPE_BUILTIN);
	out->value_type->builtin_type=value_type;
	out->binary[0]=left;
	out->binary[1]=right;
	return out;
}



static _Bool _mark_read_usage_recursive(glsl_ast_node_t* node,glsl_error_t* error){
	switch (node->type){
		case GLSL_AST_NODE_TYPE_NONE:
		case GLSL_AST_NODE_TYPE_ADD_ASSIGN:
		case GLSL_AST_NODE_TYPE_AND_ASSIGN:
		case GLSL_AST_NODE_TYPE_ASSIGN:
		case GLSL_AST_NODE_TYPE_DIVIDE_ASSIGN:
		case GLSL_AST_NODE_TYPE_LEFT_SHIFT_ASSIGN:
		case GLSL_AST_NODE_TYPE_MODULO_ASSIGN:
		case GLSL_AST_NODE_TYPE_MULTIPLY_ASSIGN:
		case GLSL_AST_NODE_TYPE_OR_ASSIGN:
		case GLSL_AST_NODE_TYPE_RIGHT_SHIFT_ASSIGN:
		case GLSL_AST_NODE_TYPE_SUBTRACT_ASSIGN:
		case GLSL_AST_NODE_TYPE_XOR_ASSIGN:
			return 1;
		case GLSL_AST_NODE_TYPE_ARRAY_ACCESS:
			return 0;
		case GLSL_AST_NODE_TYPE_BIT_INVERSE:
		case GLSL_AST_NODE_TYPE_NEGATE:
		case GLSL_AST_NODE_TYPE_NOT:
			return _mark_read_usage_recursive(node->unary,error);
		case GLSL_AST_NODE_TYPE_CALL:
			return 0;
		case GLSL_AST_NODE_TYPE_CONSTRUCTOR:
			for (u32 i=0;i<node->constructor.arg_count;i++){
				if (!_mark_read_usage_recursive(node->constructor.args[i],error)){
					return 0;
				}
			}
			return 1;
		case GLSL_AST_NODE_TYPE_COMMA:
			return 0;
		case GLSL_AST_NODE_TYPE_MEMBER_ACCESS:
			return _mark_read_usage_recursive(node->member_access.value,error);
		case GLSL_AST_NODE_TYPE_VAR:
			if (!(node->var->possible_usage_flags&GLSL_AST_VAR_USAGE_FLAG_READ)){
				*error=_glsl_error_create_parser_disallowed_operation(&(node->var->storage),GLSL_AST_VAR_USAGE_FLAG_READ);
				return 0;
			}
			if (!(node->var->flags&GLSL_AST_VAR_FLAG_INITIALIZED)){
				*error=_glsl_error_create_parser_uninitialized_var(node->var->name);
				return 0;
			}
			node->var->usage_flags|=GLSL_AST_VAR_USAGE_FLAG_READ;
			return 1;
		case GLSL_AST_NODE_TYPE_VAR_BOOL:
		case GLSL_AST_NODE_TYPE_VAR_FLOAT:
		case GLSL_AST_NODE_TYPE_VAR_INT:
			return 1;
		case GLSL_AST_NODE_TYPE_SWIZZLE:
			return _mark_read_usage_recursive(node->swizzle.value,error);
	}
	return _mark_read_usage_recursive(node->binary[0],error)&&_mark_read_usage_recursive(node->binary[1],error);
}



static _Bool _check_lvalue_recursive(glsl_ast_node_t* value,glsl_error_t* error){
	if (value->type==GLSL_AST_NODE_TYPE_VAR){
		if (!(value->var->possible_usage_flags&GLSL_AST_VAR_USAGE_FLAG_WRITE)){
			*error=_glsl_error_create_parser_disallowed_operation(&(value->var->storage),GLSL_AST_VAR_USAGE_FLAG_WRITE);
			return 0;
		}
		value->var->usage_flags|=GLSL_AST_VAR_USAGE_FLAG_WRITE;
		value->var->flags|=GLSL_AST_VAR_FLAG_INITIALIZED;
		return 1;
	}
	if (value->type==GLSL_AST_NODE_TYPE_SWIZZLE){
		u32 component_bitmap=0;
		for (u32 i=0;i<value->swizzle.pattern_length;i++){
			u32 component=(value->swizzle.pattern>>(i<<1))&3;
			if (component_bitmap&(1<<component)){
				*error=_glsl_error_create_parser_dupliated_swizzle_component();
				return 0;
			}
			component_bitmap|=1<<component;
		}
		return _check_lvalue_recursive(value->swizzle.value,error);
	}
	return 0;
}



static _Bool _check_lvalue(glsl_ast_node_t* value,glsl_error_t* error){
	if (_check_lvalue_recursive(value,error)){
		return 1;
	}
	if (*error==GLSL_NO_ERROR){
		*error=_glsl_error_create_parser_invalid_lvalue();
	}
	return 0;
}



static glsl_ast_node_t* _add_unary(glsl_ast_node_t* value,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _add_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(ADD,INT,INT,INT);
	MATCH_AND_RETURN(ADD,UINT,UINT,UINT);
	MATCH_AND_RETURN(ADD,FLOAT,FLOAT,FLOAT);
	MATCH_AND_RETURN(ADD,FLOAT,VEC2,VEC2);
	MATCH_AND_RETURN(ADD,VEC2,FLOAT,VEC2);
	MATCH_AND_RETURN(ADD,FLOAT,VEC3,VEC3);
	MATCH_AND_RETURN(ADD,VEC3,FLOAT,VEC3);
	MATCH_AND_RETURN(ADD,FLOAT,VEC4,VEC4);
	MATCH_AND_RETURN(ADD,VEC4,FLOAT,VEC4);
	MATCH_AND_RETURN(ADD,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(ADD,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(ADD,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(ADD,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(ADD,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(ADD,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(ADD,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(ADD,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(ADD,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(ADD,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(ADD,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(ADD,UVEC4,UINT,UVEC4);
	MATCH_AND_RETURN(ADD,FLOAT,MAT22,MAT22);
	MATCH_AND_RETURN(ADD,MAT22,FLOAT,MAT22);
	MATCH_AND_RETURN(ADD,FLOAT,MAT23,MAT23);
	MATCH_AND_RETURN(ADD,MAT23,FLOAT,MAT23);
	MATCH_AND_RETURN(ADD,FLOAT,MAT24,MAT24);
	MATCH_AND_RETURN(ADD,MAT24,FLOAT,MAT24);
	MATCH_AND_RETURN(ADD,FLOAT,MAT32,MAT32);
	MATCH_AND_RETURN(ADD,MAT32,FLOAT,MAT32);
	MATCH_AND_RETURN(ADD,FLOAT,MAT33,MAT33);
	MATCH_AND_RETURN(ADD,MAT33,FLOAT,MAT33);
	MATCH_AND_RETURN(ADD,FLOAT,MAT34,MAT34);
	MATCH_AND_RETURN(ADD,MAT34,FLOAT,MAT34);
	MATCH_AND_RETURN(ADD,FLOAT,MAT42,MAT42);
	MATCH_AND_RETURN(ADD,MAT42,FLOAT,MAT42);
	MATCH_AND_RETURN(ADD,FLOAT,MAT43,MAT43);
	MATCH_AND_RETURN(ADD,MAT43,FLOAT,MAT43);
	MATCH_AND_RETURN(ADD,FLOAT,MAT44,MAT44);
	MATCH_AND_RETURN(ADD,MAT44,FLOAT,MAT44);
	MATCH_AND_RETURN(ADD,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(ADD,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(ADD,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(ADD,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(ADD,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(ADD,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(ADD,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(ADD,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(ADD,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(ADD,MAT22,MAT22,MAT22);
	MATCH_AND_RETURN(ADD,MAT23,MAT23,MAT23);
	MATCH_AND_RETURN(ADD,MAT24,MAT24,MAT24);
	MATCH_AND_RETURN(ADD,MAT32,MAT32,MAT32);
	MATCH_AND_RETURN(ADD,MAT33,MAT33,MAT33);
	MATCH_AND_RETURN(ADD,MAT34,MAT34,MAT34);
	MATCH_AND_RETURN(ADD,MAT42,MAT42,MAT42);
	MATCH_AND_RETURN(ADD,MAT43,MAT43,MAT43);
	MATCH_AND_RETURN(ADD,MAT44,MAT44,MAT44);
	return NULL;
}



static glsl_ast_node_t* _add_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _and_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(AND,INT,INT,INT);
	MATCH_AND_RETURN(AND,UINT,UINT,UINT);
	MATCH_AND_RETURN(AND,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(AND,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(AND,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(AND,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(AND,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(AND,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(AND,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(AND,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(AND,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(AND,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(AND,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(AND,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(AND,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(AND,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(AND,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(AND,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(AND,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(AND,UVEC4,UINT,UVEC4);
	return NULL;
}



static glsl_ast_node_t* _div_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(DIVIDE,INT,INT,INT);
	MATCH_AND_RETURN(DIVIDE,UINT,UINT,UINT);
	MATCH_AND_RETURN(DIVIDE,FLOAT,FLOAT,FLOAT);
	MATCH_AND_RETURN(DIVIDE,FLOAT,VEC2,VEC2);
	MATCH_AND_RETURN(DIVIDE,VEC2,FLOAT,VEC2);
	MATCH_AND_RETURN(DIVIDE,FLOAT,VEC3,VEC3);
	MATCH_AND_RETURN(DIVIDE,VEC3,FLOAT,VEC3);
	MATCH_AND_RETURN(DIVIDE,FLOAT,VEC4,VEC4);
	MATCH_AND_RETURN(DIVIDE,VEC4,FLOAT,VEC4);
	MATCH_AND_RETURN(DIVIDE,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(DIVIDE,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(DIVIDE,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(DIVIDE,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(DIVIDE,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(DIVIDE,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(DIVIDE,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(DIVIDE,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(DIVIDE,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(DIVIDE,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(DIVIDE,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(DIVIDE,UVEC4,UINT,UVEC4);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT22,MAT22);
	MATCH_AND_RETURN(DIVIDE,MAT22,FLOAT,MAT22);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT23,MAT23);
	MATCH_AND_RETURN(DIVIDE,MAT23,FLOAT,MAT23);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT24,MAT24);
	MATCH_AND_RETURN(DIVIDE,MAT24,FLOAT,MAT24);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT32,MAT32);
	MATCH_AND_RETURN(DIVIDE,MAT32,FLOAT,MAT32);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT33,MAT33);
	MATCH_AND_RETURN(DIVIDE,MAT33,FLOAT,MAT33);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT34,MAT34);
	MATCH_AND_RETURN(DIVIDE,MAT34,FLOAT,MAT34);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT42,MAT42);
	MATCH_AND_RETURN(DIVIDE,MAT42,FLOAT,MAT42);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT43,MAT43);
	MATCH_AND_RETURN(DIVIDE,MAT43,FLOAT,MAT43);
	MATCH_AND_RETURN(DIVIDE,FLOAT,MAT44,MAT44);
	MATCH_AND_RETURN(DIVIDE,MAT44,FLOAT,MAT44);
	MATCH_AND_RETURN(DIVIDE,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(DIVIDE,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(DIVIDE,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(DIVIDE,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(DIVIDE,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(DIVIDE,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(DIVIDE,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(DIVIDE,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(DIVIDE,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(DIVIDE,MAT22,MAT22,MAT22);
	MATCH_AND_RETURN(DIVIDE,MAT23,MAT23,MAT23);
	MATCH_AND_RETURN(DIVIDE,MAT24,MAT24,MAT24);
	MATCH_AND_RETURN(DIVIDE,MAT32,MAT32,MAT32);
	MATCH_AND_RETURN(DIVIDE,MAT33,MAT33,MAT33);
	MATCH_AND_RETURN(DIVIDE,MAT34,MAT34,MAT34);
	MATCH_AND_RETURN(DIVIDE,MAT42,MAT42,MAT42);
	MATCH_AND_RETURN(DIVIDE,MAT43,MAT43,MAT43);
	MATCH_AND_RETURN(DIVIDE,MAT44,MAT44,MAT44);
	return NULL;
}



static glsl_ast_node_t* _div_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _equ_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(EQUALS,BOOL,BOOL,BOOL);
	MATCH_AND_RETURN(EQUALS,INT,INT,BOOL);
	MATCH_AND_RETURN(EQUALS,UINT,UINT,BOOL);
	MATCH_AND_RETURN(EQUALS,FLOAT,FLOAT,BOOL);
	MATCH_AND_RETURN(EQUALS,VEC2,VEC2,BOOL);
	MATCH_AND_RETURN(EQUALS,VEC3,VEC3,BOOL);
	MATCH_AND_RETURN(EQUALS,VEC4,VEC4,BOOL);
	MATCH_AND_RETURN(EQUALS,IVEC2,IVEC2,BOOL);
	MATCH_AND_RETURN(EQUALS,IVEC3,IVEC3,BOOL);
	MATCH_AND_RETURN(EQUALS,IVEC4,IVEC4,BOOL);
	MATCH_AND_RETURN(EQUALS,UVEC2,UVEC2,BOOL);
	MATCH_AND_RETURN(EQUALS,UVEC3,UVEC3,BOOL);
	MATCH_AND_RETURN(EQUALS,UVEC4,UVEC4,BOOL);
	MATCH_AND_RETURN(EQUALS,BVEC2,BVEC2,BOOL);
	MATCH_AND_RETURN(EQUALS,BVEC3,BVEC3,BOOL);
	MATCH_AND_RETURN(EQUALS,BVEC4,BVEC4,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT22,MAT22,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT23,MAT23,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT24,MAT24,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT32,MAT32,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT33,MAT33,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT34,MAT34,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT42,MAT42,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT43,MAT43,BOOL);
	MATCH_AND_RETURN(EQUALS,MAT44,MAT44,BOOL);
	return NULL;
}



static glsl_ast_node_t* _equal_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	if (!_check_lvalue(left,error)||!_mark_read_usage_recursive(right,error)){
		return NULL;
	}
	MATCH_AND_RETURN(ASSIGN,BOOL,BOOL,BOOL);
	MATCH_AND_RETURN(ASSIGN,INT,INT,INT);
	MATCH_AND_RETURN(ASSIGN,UINT,UINT,UINT);
	MATCH_AND_RETURN(ASSIGN,FLOAT,FLOAT,FLOAT);
	MATCH_AND_RETURN(ASSIGN,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(ASSIGN,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(ASSIGN,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(ASSIGN,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(ASSIGN,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(ASSIGN,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(ASSIGN,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(ASSIGN,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(ASSIGN,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(ASSIGN,BVEC2,BVEC2,BVEC2);
	MATCH_AND_RETURN(ASSIGN,BVEC3,BVEC3,BVEC3);
	MATCH_AND_RETURN(ASSIGN,BVEC4,BVEC4,BVEC4);
	MATCH_AND_RETURN(ASSIGN,MAT22,MAT22,MAT22);
	MATCH_AND_RETURN(ASSIGN,MAT23,MAT23,MAT23);
	MATCH_AND_RETURN(ASSIGN,MAT24,MAT24,MAT24);
	MATCH_AND_RETURN(ASSIGN,MAT32,MAT22,MAT32);
	MATCH_AND_RETURN(ASSIGN,MAT33,MAT23,MAT33);
	MATCH_AND_RETURN(ASSIGN,MAT34,MAT24,MAT34);
	MATCH_AND_RETURN(ASSIGN,MAT42,MAT22,MAT42);
	MATCH_AND_RETURN(ASSIGN,MAT43,MAT23,MAT43);
	MATCH_AND_RETURN(ASSIGN,MAT44,MAT24,MAT44);
	return NULL;
}



static glsl_ast_node_t* _geq_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(NOT_LESS_THAN,INT,INT,BOOL);
	MATCH_AND_RETURN(NOT_LESS_THAN,UINT,UINT,BOOL);
	MATCH_AND_RETURN(NOT_LESS_THAN,FLOAT,FLOAT,BOOL);
	return NULL;
}



static glsl_ast_node_t* _inv_unary(glsl_ast_node_t* value,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _land_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(LOGICAL_AND,BOOL,BOOL,BOOL);
	return NULL;
}



static glsl_ast_node_t* _leq_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(NOT_GREATER_THAN,INT,INT,BOOL);
	MATCH_AND_RETURN(NOT_GREATER_THAN,UINT,UINT,BOOL);
	MATCH_AND_RETURN(NOT_GREATER_THAN,FLOAT,FLOAT,BOOL);
	return NULL;
}



static glsl_ast_node_t* _less_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(LESS_THAN,INT,INT,BOOL);
	MATCH_AND_RETURN(LESS_THAN,UINT,UINT,BOOL);
	MATCH_AND_RETURN(LESS_THAN,FLOAT,FLOAT,BOOL);
	return NULL;
}



static glsl_ast_node_t* _lor_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(LOGICAL_OR,BOOL,BOOL,BOOL);
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _lsh_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(LEFT_SHIFT,INT,INT,INT);
	MATCH_AND_RETURN(LEFT_SHIFT,INT,UINT,INT);
	MATCH_AND_RETURN(LEFT_SHIFT,UINT,UINT,UINT);
	MATCH_AND_RETURN(LEFT_SHIFT,UINT,INT,UINT);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC2,INT,VEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC2,UINT,VEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC2,INT,UVEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC2,UVEC2,VEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC2,VEC2,UVEC2);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC3,INT,VEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC3,UINT,VEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC3,INT,UVEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC3,UVEC3,VEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC3,VEC3,UVEC3);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC4,INT,VEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC4,UINT,VEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC4,UINT,UVEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC4,INT,UVEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,VEC4,UVEC4,VEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(LEFT_SHIFT,UVEC4,VEC4,UVEC4);
	return NULL;
}



static glsl_ast_node_t* _lsh_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _mod_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(MODULO,INT,INT,INT);
	MATCH_AND_RETURN(MODULO,UINT,UINT,UINT);
	MATCH_AND_RETURN(MODULO,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(MODULO,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(MODULO,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(MODULO,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(MODULO,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(MODULO,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(MODULO,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(MODULO,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(MODULO,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(MODULO,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(MODULO,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(MODULO,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(MODULO,UINT,UVEC2,IVEC2);
	MATCH_AND_RETURN(MODULO,UVEC2,UINT,IVEC2);
	MATCH_AND_RETURN(MODULO,UINT,UVEC3,IVEC3);
	MATCH_AND_RETURN(MODULO,UVEC3,UINT,IVEC3);
	MATCH_AND_RETURN(MODULO,UINT,UVEC4,IVEC4);
	MATCH_AND_RETURN(MODULO,UVEC4,UINT,IVEC4);
	return NULL;
}



static glsl_ast_node_t* _mod_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _more_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(GREATER_THAN,INT,INT,BOOL);
	MATCH_AND_RETURN(GREATER_THAN,UINT,UINT,BOOL);
	MATCH_AND_RETURN(GREATER_THAN,FLOAT,FLOAT,BOOL);
	return NULL;
}



static glsl_ast_node_t* _mul_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(MULTIPLY,INT,INT,INT);
	MATCH_AND_RETURN(MULTIPLY,UINT,UINT,UINT);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,FLOAT,FLOAT);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,VEC2,VEC2);
	MATCH_AND_RETURN(MULTIPLY,VEC2,FLOAT,VEC2);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,VEC3,VEC3);
	MATCH_AND_RETURN(MULTIPLY,VEC3,FLOAT,VEC3);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,VEC4,VEC4);
	MATCH_AND_RETURN(MULTIPLY,VEC4,FLOAT,VEC4);
	MATCH_AND_RETURN(MULTIPLY,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(MULTIPLY,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(MULTIPLY,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(MULTIPLY,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(MULTIPLY,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(MULTIPLY,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(MULTIPLY,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(MULTIPLY,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(MULTIPLY,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(MULTIPLY,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(MULTIPLY,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(MULTIPLY,UVEC4,UINT,UVEC4);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT22,MAT22);
	MATCH_AND_RETURN(MULTIPLY,MAT22,FLOAT,MAT22);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT23,MAT23);
	MATCH_AND_RETURN(MULTIPLY,MAT23,FLOAT,MAT23);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT24,MAT24);
	MATCH_AND_RETURN(MULTIPLY,MAT24,FLOAT,MAT24);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT32,MAT32);
	MATCH_AND_RETURN(MULTIPLY,MAT32,FLOAT,MAT32);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT33,MAT33);
	MATCH_AND_RETURN(MULTIPLY,MAT33,FLOAT,MAT33);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT34,MAT34);
	MATCH_AND_RETURN(MULTIPLY,MAT34,FLOAT,MAT34);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT42,MAT42);
	MATCH_AND_RETURN(MULTIPLY,MAT42,FLOAT,MAT42);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT43,MAT43);
	MATCH_AND_RETURN(MULTIPLY,MAT43,FLOAT,MAT43);
	MATCH_AND_RETURN(MULTIPLY,FLOAT,MAT44,MAT44);
	MATCH_AND_RETURN(MULTIPLY,MAT44,FLOAT,MAT44);
	MATCH_AND_RETURN(MULTIPLY,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(MULTIPLY,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(MULTIPLY,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(MULTIPLY,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(MULTIPLY,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(MULTIPLY,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(MULTIPLY,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(MULTIPLY,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(MULTIPLY,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(MULTIPLY,VEC2,VEC3,MAT32);
	MATCH_AND_RETURN(MULTIPLY,VEC2,VEC4,MAT42);
	MATCH_AND_RETURN(MULTIPLY,VEC3,VEC2,MAT23);
	MATCH_AND_RETURN(MULTIPLY,VEC3,VEC4,MAT43);
	MATCH_AND_RETURN(MULTIPLY,VEC4,VEC2,MAT24);
	MATCH_AND_RETURN(MULTIPLY,VEC4,VEC3,MAT34);
	MATCH_AND_RETURN(MULTIPLY,MAT22,VEC2,VEC2);
	MATCH_AND_RETURN(MULTIPLY,MAT23,VEC2,VEC3);
	MATCH_AND_RETURN(MULTIPLY,MAT24,VEC2,VEC4);
	MATCH_AND_RETURN(MULTIPLY,MAT32,VEC3,VEC2);
	MATCH_AND_RETURN(MULTIPLY,MAT33,VEC3,VEC3);
	MATCH_AND_RETURN(MULTIPLY,MAT34,VEC3,VEC4);
	MATCH_AND_RETURN(MULTIPLY,MAT42,VEC4,VEC2);
	MATCH_AND_RETURN(MULTIPLY,MAT43,VEC4,VEC3);
	MATCH_AND_RETURN(MULTIPLY,MAT44,VEC4,VEC4);
	return NULL;
}



static glsl_ast_node_t* _mul_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _neq_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(NOT_EQUALS,BOOL,BOOL,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,INT,INT,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,UINT,UINT,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,FLOAT,FLOAT,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,VEC2,VEC2,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,VEC3,VEC3,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,VEC4,VEC4,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,IVEC2,IVEC2,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,IVEC3,IVEC3,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,IVEC4,IVEC4,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,UVEC2,UVEC2,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,UVEC3,UVEC3,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,UVEC4,UVEC4,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,BVEC2,BVEC2,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,BVEC3,BVEC3,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,BVEC4,BVEC4,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT22,MAT22,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT23,MAT23,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT24,MAT24,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT32,MAT32,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT33,MAT33,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT34,MAT34,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT42,MAT42,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT43,MAT43,BOOL);
	MATCH_AND_RETURN(NOT_EQUALS,MAT44,MAT44,BOOL);
	return NULL;
}



static glsl_ast_node_t* _not_unary(glsl_ast_node_t* value,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _or_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(OR,INT,INT,INT);
	MATCH_AND_RETURN(OR,UINT,UINT,UINT);
	MATCH_AND_RETURN(OR,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(OR,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(OR,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(OR,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(OR,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(OR,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(OR,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(OR,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(OR,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(OR,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(OR,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(OR,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(OR,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(OR,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(OR,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(OR,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(OR,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(OR,UVEC4,UINT,UVEC4);
	return NULL;
}



static glsl_ast_node_t* _rsh_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(RIGHT_SHIFT,INT,INT,INT);
	MATCH_AND_RETURN(RIGHT_SHIFT,INT,UINT,INT);
	MATCH_AND_RETURN(RIGHT_SHIFT,UINT,UINT,UINT);
	MATCH_AND_RETURN(RIGHT_SHIFT,UINT,INT,UINT);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC2,INT,VEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC2,UINT,VEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC2,INT,UVEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC2,UVEC2,VEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC2,VEC2,UVEC2);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC3,INT,VEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC3,UINT,VEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC3,INT,UVEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC3,UVEC3,VEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC3,VEC3,UVEC3);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC4,INT,VEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC4,UINT,VEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC4,UINT,UVEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC4,INT,UVEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,VEC4,UVEC4,VEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(RIGHT_SHIFT,UVEC4,VEC4,UVEC4);
	return NULL;
}



static glsl_ast_node_t* _rsh_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _sub_unary(glsl_ast_node_t* value,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _sub_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(SUBTRACT,INT,INT,INT);
	MATCH_AND_RETURN(SUBTRACT,UINT,UINT,UINT);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,FLOAT,FLOAT);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,VEC2,VEC2);
	MATCH_AND_RETURN(SUBTRACT,VEC2,FLOAT,VEC2);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,VEC3,VEC3);
	MATCH_AND_RETURN(SUBTRACT,VEC3,FLOAT,VEC3);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,VEC4,VEC4);
	MATCH_AND_RETURN(SUBTRACT,VEC4,FLOAT,VEC4);
	MATCH_AND_RETURN(SUBTRACT,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(SUBTRACT,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(SUBTRACT,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(SUBTRACT,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(SUBTRACT,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(SUBTRACT,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(SUBTRACT,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(SUBTRACT,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(SUBTRACT,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(SUBTRACT,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(SUBTRACT,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(SUBTRACT,UVEC4,UINT,UVEC4);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT22,MAT22);
	MATCH_AND_RETURN(SUBTRACT,MAT22,FLOAT,MAT22);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT23,MAT23);
	MATCH_AND_RETURN(SUBTRACT,MAT23,FLOAT,MAT23);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT24,MAT24);
	MATCH_AND_RETURN(SUBTRACT,MAT24,FLOAT,MAT24);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT32,MAT32);
	MATCH_AND_RETURN(SUBTRACT,MAT32,FLOAT,MAT32);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT33,MAT33);
	MATCH_AND_RETURN(SUBTRACT,MAT33,FLOAT,MAT33);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT34,MAT34);
	MATCH_AND_RETURN(SUBTRACT,MAT34,FLOAT,MAT34);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT42,MAT42);
	MATCH_AND_RETURN(SUBTRACT,MAT42,FLOAT,MAT42);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT43,MAT43);
	MATCH_AND_RETURN(SUBTRACT,MAT43,FLOAT,MAT43);
	MATCH_AND_RETURN(SUBTRACT,FLOAT,MAT44,MAT44);
	MATCH_AND_RETURN(SUBTRACT,MAT44,FLOAT,MAT44);
	MATCH_AND_RETURN(SUBTRACT,VEC2,VEC2,VEC2);
	MATCH_AND_RETURN(SUBTRACT,VEC3,VEC3,VEC3);
	MATCH_AND_RETURN(SUBTRACT,VEC4,VEC4,VEC4);
	MATCH_AND_RETURN(SUBTRACT,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(SUBTRACT,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(SUBTRACT,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(SUBTRACT,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(SUBTRACT,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(SUBTRACT,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(SUBTRACT,MAT22,MAT22,MAT22);
	MATCH_AND_RETURN(SUBTRACT,MAT23,MAT23,MAT23);
	MATCH_AND_RETURN(SUBTRACT,MAT24,MAT24,MAT24);
	MATCH_AND_RETURN(SUBTRACT,MAT32,MAT32,MAT32);
	MATCH_AND_RETURN(SUBTRACT,MAT33,MAT33,MAT33);
	MATCH_AND_RETURN(SUBTRACT,MAT34,MAT34,MAT34);
	MATCH_AND_RETURN(SUBTRACT,MAT42,MAT42,MAT42);
	MATCH_AND_RETURN(SUBTRACT,MAT43,MAT43,MAT43);
	MATCH_AND_RETURN(SUBTRACT,MAT44,MAT44,MAT44);
	return NULL;
}



static glsl_ast_node_t* _sub_assign_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	*error=_glsl_error_create_unimplemented(__FILE__,__LINE__,__func__);
	return NULL;
}



static glsl_ast_node_t* _xor_binary(glsl_ast_node_t* left,glsl_ast_node_t* right,glsl_error_t* error){
	MATCH_AND_RETURN(XOR,INT,INT,INT);
	MATCH_AND_RETURN(XOR,UINT,UINT,UINT);
	MATCH_AND_RETURN(XOR,IVEC2,IVEC2,IVEC2);
	MATCH_AND_RETURN(XOR,IVEC3,IVEC3,IVEC3);
	MATCH_AND_RETURN(XOR,IVEC4,IVEC4,IVEC4);
	MATCH_AND_RETURN(XOR,INT,IVEC2,IVEC2);
	MATCH_AND_RETURN(XOR,IVEC2,INT,IVEC2);
	MATCH_AND_RETURN(XOR,INT,IVEC3,IVEC3);
	MATCH_AND_RETURN(XOR,IVEC3,INT,IVEC3);
	MATCH_AND_RETURN(XOR,INT,IVEC4,IVEC4);
	MATCH_AND_RETURN(XOR,IVEC4,INT,IVEC4);
	MATCH_AND_RETURN(XOR,UVEC2,UVEC2,UVEC2);
	MATCH_AND_RETURN(XOR,UVEC3,UVEC3,UVEC3);
	MATCH_AND_RETURN(XOR,UVEC4,UVEC4,UVEC4);
	MATCH_AND_RETURN(XOR,UINT,UVEC2,UVEC2);
	MATCH_AND_RETURN(XOR,UVEC2,UINT,UVEC2);
	MATCH_AND_RETURN(XOR,UINT,UVEC3,UVEC3);
	MATCH_AND_RETURN(XOR,UVEC3,UINT,UVEC3);
	MATCH_AND_RETURN(XOR,UINT,UVEC4,UVEC4);
	MATCH_AND_RETURN(XOR,UVEC4,UINT,UVEC4);
	return NULL;
}



const glsl_operator_t _glsl_operator_table[GLSL_LEXER_TOKEN_MAX_TYPE+1]={
	[GLSL_LEXER_TOKEN_TYPE_ADD]={"+",2,4,_add_unary,_add_binary},
	[GLSL_LEXER_TOKEN_TYPE_ADD_ASSIGN]={"+=",0,15,NULL,_add_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_AND]={"&",0,8,NULL,_and_binary},
	[GLSL_LEXER_TOKEN_TYPE_DEC]={"--",0,0,NULL,NULL},
	[GLSL_LEXER_TOKEN_TYPE_DIV]={"/",0,3,NULL,_div_binary},
	[GLSL_LEXER_TOKEN_TYPE_DIV_ASSIGN]={"/=",0,15,NULL,_div_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_EQU]={"==",0,7,NULL,_equ_binary},
	[GLSL_LEXER_TOKEN_TYPE_EQUAL]={"=",0,15,NULL,_equal_binary},
	[GLSL_LEXER_TOKEN_TYPE_GEQ]={">=",0,6,NULL,_geq_binary},
	[GLSL_LEXER_TOKEN_TYPE_INC]={"++",0,0,NULL,NULL},
	[GLSL_LEXER_TOKEN_TYPE_INV]={"~",2,0,_inv_unary,NULL},
	[GLSL_LEXER_TOKEN_TYPE_LAND]={"&&",0,11,NULL,_land_binary},
	[GLSL_LEXER_TOKEN_TYPE_LEQ]={"<=",0,6,NULL,_leq_binary},
	[GLSL_LEXER_TOKEN_TYPE_LESS]={"<",0,6,NULL,_less_binary},
	[GLSL_LEXER_TOKEN_TYPE_LOR]={"||",0,13,NULL,_lor_binary},
	[GLSL_LEXER_TOKEN_TYPE_LSH]={"<<",0,5,NULL,_lsh_binary},
	[GLSL_LEXER_TOKEN_TYPE_LSH_ASSIGN]={"<<=",0,15,NULL,_lsh_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_MOD]={"%",0,3,NULL,_mod_binary},
	[GLSL_LEXER_TOKEN_TYPE_MOD_ASSIGN]={"%=",0,15,NULL,_mod_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_MORE]={">",0,6,NULL,_more_binary},
	[GLSL_LEXER_TOKEN_TYPE_MUL]={"*",0,3,NULL,_mul_binary},
	[GLSL_LEXER_TOKEN_TYPE_MUL_ASSIGN]={"*=",0,15,NULL,_mul_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_NEQ]={"!=",0,7,NULL,_neq_binary},
	[GLSL_LEXER_TOKEN_TYPE_NOT]={"!",2,0,_not_unary,NULL},
	[GLSL_LEXER_TOKEN_TYPE_OR]={"|",0,10,NULL,_or_binary},
	[GLSL_LEXER_TOKEN_TYPE_QUESTION_MARK]={"?",0,0,NULL,NULL},
	[GLSL_LEXER_TOKEN_TYPE_RSH]={">>",0,5,NULL,_rsh_binary},
	[GLSL_LEXER_TOKEN_TYPE_RSH_ASSIGN]={">>=",0,15,NULL,_rsh_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_SUB]={"-",2,4,_sub_unary,_sub_binary},
	[GLSL_LEXER_TOKEN_TYPE_SUB_ASSIGN]={"-=",0,15,NULL,_sub_assign_binary},
	[GLSL_LEXER_TOKEN_TYPE_XOR]={"^",0,9,NULL,_xor_binary},
};
