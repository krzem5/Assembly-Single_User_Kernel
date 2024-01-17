#ifndef _GLSL_LEXER_H_
#define _GLSL_LEXER_H_ 1
#include <glsl/builtin_types.h>
#include <glsl/error.h>
#include <sys/types.h>



#define GLSL_LEXER_TOKEN_TYPE_NONE 0
#define GLSL_LEXER_TOKEN_TYPE_ADD 1
#define GLSL_LEXER_TOKEN_TYPE_ADD_ASSIGN 2
#define GLSL_LEXER_TOKEN_TYPE_AND 3
#define GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE 4
#define GLSL_LEXER_TOKEN_TYPE_BOOL 5
#define GLSL_LEXER_TOKEN_TYPE_BREAK 6
#define GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE 7
#define GLSL_LEXER_TOKEN_TYPE_CASE 8
#define GLSL_LEXER_TOKEN_TYPE_CENTROID 9
#define GLSL_LEXER_TOKEN_TYPE_COLON 10
#define GLSL_LEXER_TOKEN_TYPE_COMMA 11
#define GLSL_LEXER_TOKEN_TYPE_CONST 12
#define GLSL_LEXER_TOKEN_TYPE_CONST_BOOL 13
#define GLSL_LEXER_TOKEN_TYPE_CONST_FLOAT 14
#define GLSL_LEXER_TOKEN_TYPE_CONST_INT 15
#define GLSL_LEXER_TOKEN_TYPE_CONTINUE 16
#define GLSL_LEXER_TOKEN_TYPE_DEC 17
#define GLSL_LEXER_TOKEN_TYPE_DEFAULT 18
#define GLSL_LEXER_TOKEN_TYPE_DISCARD 19
#define GLSL_LEXER_TOKEN_TYPE_DIV 20
#define GLSL_LEXER_TOKEN_TYPE_DIV_ASSIGN 21
#define GLSL_LEXER_TOKEN_TYPE_DO 22
#define GLSL_LEXER_TOKEN_TYPE_ELSE 23
#define GLSL_LEXER_TOKEN_TYPE_EQU 24
#define GLSL_LEXER_TOKEN_TYPE_EQUAL 25
#define GLSL_LEXER_TOKEN_TYPE_FLAT 26
#define GLSL_LEXER_TOKEN_TYPE_FLOAT 27
#define GLSL_LEXER_TOKEN_TYPE_FOR 28
#define GLSL_LEXER_TOKEN_TYPE_GEQ 29
#define GLSL_LEXER_TOKEN_TYPE_HIGHP 30
#define GLSL_LEXER_TOKEN_TYPE_IDENTIFIER 31
#define GLSL_LEXER_TOKEN_TYPE_IF 32
#define GLSL_LEXER_TOKEN_TYPE_IN 33
#define GLSL_LEXER_TOKEN_TYPE_INC 34
#define GLSL_LEXER_TOKEN_TYPE_INOUT 35
#define GLSL_LEXER_TOKEN_TYPE_INT 36
#define GLSL_LEXER_TOKEN_TYPE_INV 37
#define GLSL_LEXER_TOKEN_TYPE_INVARIANT 38
#define GLSL_LEXER_TOKEN_TYPE_LAND 39
#define GLSL_LEXER_TOKEN_TYPE_LAYOUT 40
#define GLSL_LEXER_TOKEN_TYPE_LEFT_BRACE 41
#define GLSL_LEXER_TOKEN_TYPE_LEFT_BRACKET 42
#define GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN 43
#define GLSL_LEXER_TOKEN_TYPE_LEQ 44
#define GLSL_LEXER_TOKEN_TYPE_LESS 45
#define GLSL_LEXER_TOKEN_TYPE_LOR 46
#define GLSL_LEXER_TOKEN_TYPE_LOWP 47
#define GLSL_LEXER_TOKEN_TYPE_LSH 48
#define GLSL_LEXER_TOKEN_TYPE_LSH_ASSIGN 49
#define GLSL_LEXER_TOKEN_TYPE_MEDIUMP 50
#define GLSL_LEXER_TOKEN_TYPE_MOD 51
#define GLSL_LEXER_TOKEN_TYPE_MOD_ASSIGN 52
#define GLSL_LEXER_TOKEN_TYPE_MORE 53
#define GLSL_LEXER_TOKEN_TYPE_MUL 54
#define GLSL_LEXER_TOKEN_TYPE_MUL_ASSIGN 55
#define GLSL_LEXER_TOKEN_TYPE_NEQ 56
#define GLSL_LEXER_TOKEN_TYPE_NOPERSPECTIVE 57
#define GLSL_LEXER_TOKEN_TYPE_NOT 58
#define GLSL_LEXER_TOKEN_TYPE_OR 59
#define GLSL_LEXER_TOKEN_TYPE_OUT 60
#define GLSL_LEXER_TOKEN_TYPE_PERIOD 61
#define GLSL_LEXER_TOKEN_TYPE_PRECISION 62
#define GLSL_LEXER_TOKEN_TYPE_QUESTION_MARK 63
#define GLSL_LEXER_TOKEN_TYPE_RETURN 64
#define GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACE 65
#define GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACKET 66
#define GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN 67
#define GLSL_LEXER_TOKEN_TYPE_RSH 68
#define GLSL_LEXER_TOKEN_TYPE_RSH_ASSIGN 69
#define GLSL_LEXER_TOKEN_TYPE_SEMICOLON 70
#define GLSL_LEXER_TOKEN_TYPE_SMOOTH 71
#define GLSL_LEXER_TOKEN_TYPE_STRUCT 72
#define GLSL_LEXER_TOKEN_TYPE_SUB 73
#define GLSL_LEXER_TOKEN_TYPE_SUB_ASSIGN 74
#define GLSL_LEXER_TOKEN_TYPE_SWITCH 75
#define GLSL_LEXER_TOKEN_TYPE_UNIFORM 76
#define GLSL_LEXER_TOKEN_TYPE_VARYING 77
#define GLSL_LEXER_TOKEN_TYPE_VOID 78
#define GLSL_LEXER_TOKEN_TYPE_WHILE 79
#define GLSL_LEXER_TOKEN_TYPE_XOR 80

#define GLSL_LEXER_TOKEN_MAX_TYPE GLSL_LEXER_TOKEN_TYPE_XOR



typedef u32 glsl_lexer_token_type_t;



typedef struct _GLSL_LEXER_TOKEN{
	glsl_lexer_token_type_t type;
	union{
		_Bool bool_;
		u64 int_;
		double float_;
		char* string;
		glsl_builtin_type_t builtin_type;
	};
} glsl_lexer_token_t;



typedef struct _GLSL_LEXER_TOKEN_LIST{
	glsl_lexer_token_t* data;
	u32 length;
	u32 _capacity;
} glsl_lexer_token_list_t;



glsl_error_t glsl_lexer_extract_tokens(const char* src,glsl_lexer_token_list_t* out);



void glsl_lexer_delete_token_list(glsl_lexer_token_list_t* token_list);



#endif
