#include <glsl/ast.h>
#include <glsl/error.h>
#include <glsl/lexer.h>
#include <glsl/parser.h>
#include <glsl/shader.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/glsl_common.h>
#include <test/test.h>



static glsl_error_t _execute_parser(const char* src,glsl_shader_type_t shader_type,glsl_ast_t* out){
	glsl_lexer_token_list_t token_list;
	TEST_ASSERT(!glsl_lexer_extract_tokens(src,&token_list));
	glsl_error_t error=glsl_parser_parse_tokens(&token_list,shader_type,out);
	glsl_lexer_delete_token_list(&token_list);
	return error;
}



static _Bool _check_var_storage(const glsl_ast_t* ast,const char* name,u8 storage_type,u8 storage_flags,u16 storage_layout_location,glsl_ast_block_t* storage_block){
	const glsl_ast_var_t* var=NULL;
	for (u32 i=0;i<ast->var_count;i++){
		if (!sys_string_compare(ast->vars[i]->name,name)){
			var=ast->vars[i];
			break;
		}
	}
	_Bool out=0;
	if (var){
		out=(var->storage.type==storage_type&&var->storage.flags==storage_flags&&var->storage.block==storage_block);
		if (out&&(var->storage.flags&GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION)&&var->storage.layout_location!=storage_layout_location){
			out=0;
		}
	}
	return out;
}



void test_glsl_parser(void){
	TEST_MODULE("glsl_parser");
	TEST_FUNC("glsl_parser_parse_tokens");
	TEST_GROUP("empty input");
	glsl_ast_t ast;
	TEST_ASSERT(!_execute_parser("",GLSL_SHADER_TYPE_ANY,&ast));
	glsl_ast_delete(&ast);
	TEST_GROUP("precision");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision",GLSL_SHADER_TYPE_ANY,&ast),"Expected precision qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision 10.5",GLSL_SHADER_TYPE_ANY,&ast),"Expected precision qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision lowp",GLSL_SHADER_TYPE_ANY,&ast),"Expected type, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision mediump",GLSL_SHADER_TYPE_ANY,&ast),"Expected type, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision highp",GLSL_SHADER_TYPE_ANY,&ast),"Expected type, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("precision lowp int",GLSL_SHADER_TYPE_ANY,&ast),"Expected semicolon, got ???"));
	TEST_ASSERT(!_execute_parser("precision mediump float;",GLSL_SHADER_TYPE_ANY,&ast));
	glsl_ast_delete(&ast);
	TEST_GROUP("storage");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout",GLSL_SHADER_TYPE_ANY,&ast),"Expected layout qualifiers, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(",GLSL_SHADER_TYPE_ANY,&ast),"Expected layout qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(<<=",GLSL_SHADER_TYPE_ANY,&ast),"Expected layout qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(key",GLSL_SHADER_TYPE_ANY,&ast),"Expected end of layout qualifiers, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(key=value",GLSL_SHADER_TYPE_ANY,&ast),"Expected integer constant, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(key=2",GLSL_SHADER_TYPE_ANY,&ast),"Expected end of layout qualifiers, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(location=1,location=2)",GLSL_SHADER_TYPE_ANY,&ast),"Duplicated layout qualifier 'location'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(location,",GLSL_SHADER_TYPE_ANY,&ast),"Expected layout qualifier, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(location+",GLSL_SHADER_TYPE_ANY,&ast),"Expected end of layout qualifiers, got ???"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("layout(key)",GLSL_SHADER_TYPE_ANY,&ast),"Unknown layout qualifier 'key'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("attribute",GLSL_SHADER_TYPE_ANY,&ast),"Deprecated keyword: attribute"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(_execute_parser("varying",GLSL_SHADER_TYPE_ANY,&ast),"Deprecated keyword: varying"));
	TEST_ASSERT(!_execute_parser("int var_a=5;\ncentroid const float var_b=1.0;\nlayout (location=2) in mat4 var_c;\nlayout(location=1) centroid out bool var_d;\nuniform mat3x2 var_e;",GLSL_SHADER_TYPE_ANY,&ast));
	TEST_ASSERT(_check_var_storage(&ast,"var_a",GLSL_AST_VAR_STORAGE_TYPE_DEFAULT,0,0,NULL));
	TEST_ASSERT(_check_var_storage(&ast,"var_b",GLSL_AST_VAR_STORAGE_TYPE_CONST,GLSL_AST_VAR_STORAGE_FLAG_CENTROID,0,NULL));
	TEST_ASSERT(_check_var_storage(&ast,"var_c",GLSL_AST_VAR_STORAGE_TYPE_IN,GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION,2,NULL));
	TEST_ASSERT(_check_var_storage(&ast,"var_d",GLSL_AST_VAR_STORAGE_TYPE_OUT,GLSL_AST_VAR_STORAGE_FLAG_CENTROID|GLSL_AST_VAR_STORAGE_FLAG_HAS_LAYOUT_LOCATION,1,NULL));
	TEST_ASSERT(_check_var_storage(&ast,"var_e",GLSL_AST_VAR_STORAGE_TYPE_UNIFORM,0,0,NULL));
	glsl_ast_delete(&ast);
}