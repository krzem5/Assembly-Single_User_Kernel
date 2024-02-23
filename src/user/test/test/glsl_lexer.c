#include <glsl/lexer.h>
#include <sys/string/string.h>
#include <sys/types.h>
#include <test/glsl_common.h>
#include <test/test.h>



static void _check_one_token_builtin_type(glsl_lexer_token_list_t* token_list,glsl_builtin_type_t builtin_type){
	TEST_ASSERT(token_list->length==1);
	TEST_ASSERT(token_list->data->type==GLSL_LEXER_TOKEN_TYPE_BUILTIN_TYPE);
	TEST_ASSERT(token_list->data->builtin_type==builtin_type);
	glsl_lexer_delete_token_list(token_list);
}



static void _check_one_token_bool(glsl_lexer_token_list_t* token_list,_Bool value){
	TEST_ASSERT(token_list->length==1);
	TEST_ASSERT(token_list->data->type==GLSL_LEXER_TOKEN_TYPE_CONST_BOOL);
	TEST_ASSERT(token_list->data->bool_==value);
	glsl_lexer_delete_token_list(token_list);
}



static void _check_one_token(glsl_lexer_token_list_t* token_list,glsl_lexer_token_type_t type){
	TEST_ASSERT(token_list->length==1);
	TEST_ASSERT(token_list->data->type==type);
	glsl_lexer_delete_token_list(token_list);
}



static void _check_one_token_identifier(glsl_lexer_token_list_t* token_list,const char* value){
	TEST_ASSERT(token_list->length==1);
	TEST_ASSERT(token_list->data->type==GLSL_LEXER_TOKEN_TYPE_IDENTIFIER);
	TEST_ASSERT(!sys_string_compare(token_list->data->string,value));
	glsl_lexer_delete_token_list(token_list);
}



static void _check_one_token_int(glsl_lexer_token_list_t* token_list,u64 value){
	TEST_ASSERT(token_list->length==1);
	TEST_ASSERT(token_list->data->type==GLSL_LEXER_TOKEN_TYPE_CONST_INT);
	TEST_ASSERT(token_list->data->int_==value);
	glsl_lexer_delete_token_list(token_list);
}



static void _check_one_token_float(glsl_lexer_token_list_t* token_list,double value){
	TEST_ASSERT(token_list->length==1);
	TEST_ASSERT(token_list->data->type==GLSL_LEXER_TOKEN_TYPE_CONST_FLOAT);
	double error=token_list->data->float_-value;
	TEST_ASSERT(-0.0001f<error&&error<0.0001f);
	glsl_lexer_delete_token_list(token_list);
}



void test_glsl_lexer(void){
	TEST_MODULE("glsl_lexer");
	TEST_FUNC("glsl_lexer_extract_tokens");
	TEST_GROUP("empty input");
	glsl_lexer_token_list_t token_list;
	TEST_ASSERT(!glsl_lexer_extract_tokens("",&token_list));
	TEST_ASSERT(!token_list.length);
	glsl_lexer_delete_token_list(&token_list);
	TEST_GROUP("whitespace");
	TEST_ASSERT(!glsl_lexer_extract_tokens(" \t\n\r",&token_list));
	TEST_ASSERT(!token_list.length);
	glsl_lexer_delete_token_list(&token_list);
	TEST_GROUP("comments");
	TEST_ASSERT(!glsl_lexer_extract_tokens("//\n222",&token_list));
	_check_one_token_int(&token_list,222);
	TEST_ASSERT(!glsl_lexer_extract_tokens("123//",&token_list));
	_check_one_token_int(&token_list,123);
	TEST_ASSERT(!glsl_lexer_extract_tokens("987/*aaa*/",&token_list));
	_check_one_token_int(&token_list,987);
	TEST_ASSERT(!glsl_lexer_extract_tokens("0x123 /* xyz",&token_list));
	_check_one_token_int(&token_list,0x123);
	TEST_GROUP("reserved keywords");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("active",&token_list),"Reserved keyword: active"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("asm",&token_list),"Reserved keyword: asm"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("cast",&token_list),"Reserved keyword: cast"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("class",&token_list),"Reserved keyword: class"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("common",&token_list),"Reserved keyword: common"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("double",&token_list),"Reserved keyword: double"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("dvec2",&token_list),"Reserved keyword: dvec2"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("dvec3",&token_list),"Reserved keyword: dvec3"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("dvec4",&token_list),"Reserved keyword: dvec4"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("enum",&token_list),"Reserved keyword: enum"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("extern",&token_list),"Reserved keyword: extern"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("external",&token_list),"Reserved keyword: external"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("filter",&token_list),"Reserved keyword: filter"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("fixed",&token_list),"Reserved keyword: fixed"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("fvec2",&token_list),"Reserved keyword: fvec2"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("fvec3",&token_list),"Reserved keyword: fvec3"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("fvec4",&token_list),"Reserved keyword: fvec4"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("goto",&token_list),"Reserved keyword: goto"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("half",&token_list),"Reserved keyword: half"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("hvec2",&token_list),"Reserved keyword: hvec2"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("hvec3",&token_list),"Reserved keyword: hvec3"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("hvec4",&token_list),"Reserved keyword: hvec4"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage1D",&token_list),"Reserved keyword: iimage1D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage1DArray",&token_list),"Reserved keyword: iimage1DArray"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage2D",&token_list),"Reserved keyword: iimage2D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage2DArray",&token_list),"Reserved keyword: iimage2DArray"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimage3D",&token_list),"Reserved keyword: iimage3D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimageBuffer",&token_list),"Reserved keyword: iimageBuffer"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("iimageCube",&token_list),"Reserved keyword: iimageCube"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image1D",&token_list),"Reserved keyword: image1D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image1DArray",&token_list),"Reserved keyword: image1DArray"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image1DArrayShadow",&token_list),"Reserved keyword: image1DArrayShadow"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image1DShadow",&token_list),"Reserved keyword: image1DShadow"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image2D",&token_list),"Reserved keyword: image2D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image2DArray",&token_list),"Reserved keyword: image2DArray"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image2DArrayShadow",&token_list),"Reserved keyword: image2DArrayShadow"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image2DShadow",&token_list),"Reserved keyword: image2DShadow"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("image3D",&token_list),"Reserved keyword: image3D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("imageBuffer",&token_list),"Reserved keyword: imageBuffer"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("imageCube",&token_list),"Reserved keyword: imageCube"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("inline",&token_list),"Reserved keyword: inline"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("input",&token_list),"Reserved keyword: input"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("interface",&token_list),"Reserved keyword: interface"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("long",&token_list),"Reserved keyword: long"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("namespace",&token_list),"Reserved keyword: namespace"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("noinline",&token_list),"Reserved keyword: noinline"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("output",&token_list),"Reserved keyword: output"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("packed",&token_list),"Reserved keyword: packed"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("partition",&token_list),"Reserved keyword: partition"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("public",&token_list),"Reserved keyword: public"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("row_major",&token_list),"Reserved keyword: row_major"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("sampler3DRect",&token_list),"Reserved keyword: sampler3DRect"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("short",&token_list),"Reserved keyword: short"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("sizeof",&token_list),"Reserved keyword: sizeof"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("static",&token_list),"Reserved keyword: static"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("superp",&token_list),"Reserved keyword: superp"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("template",&token_list),"Reserved keyword: template"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("this",&token_list),"Reserved keyword: this"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("typedef",&token_list),"Reserved keyword: typedef"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage1D",&token_list),"Reserved keyword: uimage1D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage1DArray",&token_list),"Reserved keyword: uimage1DArray"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage2D",&token_list),"Reserved keyword: uimage2D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage2DArray",&token_list),"Reserved keyword: uimage2DArray"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimage3D",&token_list),"Reserved keyword: uimage3D"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimageBuffer",&token_list),"Reserved keyword: uimageBuffer"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("uimageCube",&token_list),"Reserved keyword: uimageCube"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("union",&token_list),"Reserved keyword: union"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("unsigned",&token_list),"Reserved keyword: unsigned"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("using",&token_list),"Reserved keyword: using"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("volatile",&token_list),"Reserved keyword: volatile"));
	TEST_GROUP("builtin types");
	TEST_ASSERT(!glsl_lexer_extract_tokens("int",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_INT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("void",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_VOID);
	TEST_ASSERT(!glsl_lexer_extract_tokens("bool",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_BOOL);
	TEST_ASSERT(!glsl_lexer_extract_tokens("float",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_FLOAT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT22);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT33);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT44);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat2x2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT22);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat2x3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT23);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat2x4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT24);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat3x2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT32);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat3x3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT33);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat3x4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT34);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat4x2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT42);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat4x3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT43);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mat4x4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_MAT44);
	TEST_ASSERT(!glsl_lexer_extract_tokens("vec2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_VEC2);
	TEST_ASSERT(!glsl_lexer_extract_tokens("vec3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_VEC3);
	TEST_ASSERT(!glsl_lexer_extract_tokens("vec4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_VEC4);
	TEST_ASSERT(!glsl_lexer_extract_tokens("ivec2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_IVEC2);
	TEST_ASSERT(!glsl_lexer_extract_tokens("ivec3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_IVEC3);
	TEST_ASSERT(!glsl_lexer_extract_tokens("ivec4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_IVEC4);
	TEST_ASSERT(!glsl_lexer_extract_tokens("bvec2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_BVEC2);
	TEST_ASSERT(!glsl_lexer_extract_tokens("bvec3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_BVEC3);
	TEST_ASSERT(!glsl_lexer_extract_tokens("bvec4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_BVEC4);
	TEST_ASSERT(!glsl_lexer_extract_tokens("uint",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_UINT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("uvec2",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_UVEC2);
	TEST_ASSERT(!glsl_lexer_extract_tokens("uvec3",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_UVEC3);
	TEST_ASSERT(!glsl_lexer_extract_tokens("uvec4",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_UVEC4);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler1D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_1D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler3D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_3D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("samplerCube",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_CB);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler1DShadow",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_1D_SHADOW);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DShadow",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_SHADOW);
	TEST_ASSERT(!glsl_lexer_extract_tokens("samplerCubeShadow",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_CB_SHADOW);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler1DArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_1D_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler1DArrayShadow",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_1D_ARRAY_SHADOW);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DArrayShadow",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_ARRAY_SHADOW);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler1D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_1D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler2D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_2D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler3D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_3D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isamplerCube",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_CB);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler1DArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_1D_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler2DArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_2D_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler1D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_1D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler2D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_2D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler3D",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_3D);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usamplerCube",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_CB);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler1DArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_1D_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler2DArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_2D_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DRect",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_RECT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DRectShadow",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_RECT_SHADOW);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler2DRect",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_2D_RECT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler2DRect",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_2D_RECT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("samplerBuffer",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_BUFFER);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isamplerBuffer",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_BUFFER);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usamplerBuffer",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_BUFFER);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DMS",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_MULTI_SAMPLE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler2DMS",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_2D_MULTI_SAMPLE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler2DMS",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("sampler2DMSArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_SAMPLER_2D_MULTI_SAMPLE_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("isampler2DMSArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_ISAMPLER_2D_MULTI_SAMPLE_ARRAY);
	TEST_ASSERT(!glsl_lexer_extract_tokens("usampler2DMSArray",&token_list));
	_check_one_token_builtin_type(&token_list,GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE_ARRAY);
	TEST_GROUP("keywords");
	TEST_ASSERT(!glsl_lexer_extract_tokens("attribute",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("break",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_BREAK);
	TEST_ASSERT(!glsl_lexer_extract_tokens("centroid",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_CENTROID);
	TEST_ASSERT(!glsl_lexer_extract_tokens("const",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_CONST);
	TEST_ASSERT(!glsl_lexer_extract_tokens("continue",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_CONTINUE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("discard",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_DISCARD);
	TEST_ASSERT(!glsl_lexer_extract_tokens("do",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_DO);
	TEST_ASSERT(!glsl_lexer_extract_tokens("else",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_ELSE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("flat",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_FLAT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("for",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_FOR);
	TEST_ASSERT(!glsl_lexer_extract_tokens("highp",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_HIGHP);
	TEST_ASSERT(!glsl_lexer_extract_tokens("if",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_IF);
	TEST_ASSERT(!glsl_lexer_extract_tokens("in",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_IN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("inout",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_INOUT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("invariant",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_INVARIANT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("layout",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LAYOUT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("lowp",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LOWP);
	TEST_ASSERT(!glsl_lexer_extract_tokens("mediump",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_MEDIUMP);
	TEST_ASSERT(!glsl_lexer_extract_tokens("noperspective",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_NOPERSPECTIVE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("out",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_OUT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("precision",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_PRECISION);
	TEST_ASSERT(!glsl_lexer_extract_tokens("return",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_RETURN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("smooth",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_SMOOTH);
	TEST_ASSERT(!glsl_lexer_extract_tokens("struct",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_STRUCT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("uniform",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_UNIFORM);
	TEST_ASSERT(!glsl_lexer_extract_tokens("varying",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_VARYING);
	TEST_ASSERT(!glsl_lexer_extract_tokens("while",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_WHILE);
	TEST_GROUP("booleans");
	TEST_ASSERT(!glsl_lexer_extract_tokens("true",&token_list));
	_check_one_token_bool(&token_list,1);
	TEST_ASSERT(!glsl_lexer_extract_tokens("false",&token_list));
	_check_one_token_bool(&token_list,0);
	TEST_GROUP("operators");
	TEST_ASSERT(!glsl_lexer_extract_tokens("attribute",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_ATTRIBUTE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("<<=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LSH_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens(">>=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_RSH_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("!=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_NEQ);
	TEST_ASSERT(!glsl_lexer_extract_tokens("%=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_MOD_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("&&",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LAND);
	TEST_ASSERT(!glsl_lexer_extract_tokens("*=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_MUL_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("++",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_INC);
	TEST_ASSERT(!glsl_lexer_extract_tokens("+=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_ADD_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("--",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_DEC);
	TEST_ASSERT(!glsl_lexer_extract_tokens("-=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_SUB_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("/=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_DIV_ASSIGN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("<<",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LSH);
	TEST_ASSERT(!glsl_lexer_extract_tokens("<=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LEQ);
	TEST_ASSERT(!glsl_lexer_extract_tokens("==",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_EQU);
	TEST_ASSERT(!glsl_lexer_extract_tokens(">=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_GEQ);
	TEST_ASSERT(!glsl_lexer_extract_tokens(">>",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_RSH);
	TEST_ASSERT(!glsl_lexer_extract_tokens("||",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LOR);
	TEST_ASSERT(!glsl_lexer_extract_tokens("!",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_NOT);
	TEST_ASSERT(!glsl_lexer_extract_tokens("%",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_MOD);
	TEST_ASSERT(!glsl_lexer_extract_tokens("&",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_AND);
	TEST_ASSERT(!glsl_lexer_extract_tokens("(",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LEFT_PAREN);
	TEST_ASSERT(!glsl_lexer_extract_tokens(")",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_RIGHT_PAREN);
	TEST_ASSERT(!glsl_lexer_extract_tokens("*",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_MUL);
	TEST_ASSERT(!glsl_lexer_extract_tokens("+",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_ADD);
	TEST_ASSERT(!glsl_lexer_extract_tokens(",",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_COMMA);
	TEST_ASSERT(!glsl_lexer_extract_tokens("-",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_SUB);
	TEST_ASSERT(!glsl_lexer_extract_tokens("/",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_DIV);
	TEST_ASSERT(!glsl_lexer_extract_tokens(":",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_COLON);
	TEST_ASSERT(!glsl_lexer_extract_tokens(";",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_SEMICOLON);
	TEST_ASSERT(!glsl_lexer_extract_tokens("<",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LESS);
	TEST_ASSERT(!glsl_lexer_extract_tokens("=",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_EQUAL);
	TEST_ASSERT(!glsl_lexer_extract_tokens(">",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_MORE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("?",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_QUESTION_MARK);
	TEST_ASSERT(!glsl_lexer_extract_tokens("[",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LEFT_BRACKET);
	TEST_ASSERT(!glsl_lexer_extract_tokens("]",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACKET);
	TEST_ASSERT(!glsl_lexer_extract_tokens("^",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_XOR);
	TEST_ASSERT(!glsl_lexer_extract_tokens("{",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_LEFT_BRACE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("|",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_OR);
	TEST_ASSERT(!glsl_lexer_extract_tokens("}",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_RIGHT_BRACE);
	TEST_ASSERT(!glsl_lexer_extract_tokens("~",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_INV);
	TEST_ASSERT(!glsl_lexer_extract_tokens(".",&token_list));
	_check_one_token(&token_list,GLSL_LEXER_TOKEN_TYPE_PERIOD);
	TEST_GROUP("identifiers");
	TEST_ASSERT(!glsl_lexer_extract_tokens("Abc/**/",&token_list));
	_check_one_token_identifier(&token_list,"Abc");
	TEST_ASSERT(!glsl_lexer_extract_tokens("Z88\n",&token_list));
	_check_one_token_identifier(&token_list,"Z88");
	TEST_ASSERT(!glsl_lexer_extract_tokens("aabbcc",&token_list));
	_check_one_token_identifier(&token_list,"aabbcc");
	TEST_ASSERT(!glsl_lexer_extract_tokens("zyx",&token_list));
	_check_one_token_identifier(&token_list,"zyx");
	TEST_ASSERT(!glsl_lexer_extract_tokens("_internal",&token_list));
	_check_one_token_identifier(&token_list,"_internal");
	TEST_GROUP("hexadecimal integers");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("0xg",&token_list),"Hexadecimal digit expected, got 'g'"));
	TEST_ASSERT(!glsl_lexer_extract_tokens("01237",&token_list));
	_check_one_token_int(&token_list,01237);
	TEST_ASSERT(!glsl_lexer_extract_tokens("0666u",&token_list));
	_check_one_token_int(&token_list,0666);
	TEST_ASSERT(!glsl_lexer_extract_tokens("0432U",&token_list));
	_check_one_token_int(&token_list,0432);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("01ux",&token_list),"Unexpected character 'x'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("02Uu",&token_list),"Unexpected character 'u'"));
	TEST_GROUP("octal integers");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("0z",&token_list),"Octal digit expected, got 'z'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("08",&token_list),"Octal digit expected, got '8'"));
	TEST_ASSERT(!glsl_lexer_extract_tokens("0x123aBc",&token_list));
	_check_one_token_int(&token_list,0x123aBc);
	TEST_ASSERT(!glsl_lexer_extract_tokens("0XFFFu",&token_list));
	_check_one_token_int(&token_list,0xfff);
	TEST_ASSERT(!glsl_lexer_extract_tokens("0x9a9U",&token_list));
	_check_one_token_int(&token_list,0x9a9);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("0x1ux",&token_list),"Unexpected character 'x'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("0xfUu",&token_list),"Unexpected character 'u'"));
	TEST_GROUP("decimal integers");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("11p",&token_list),"Decimal digit expected, got 'p'"));
	TEST_ASSERT(!glsl_lexer_extract_tokens("2024",&token_list));
	_check_one_token_int(&token_list,2024);
	TEST_ASSERT(!glsl_lexer_extract_tokens("123u",&token_list));
	_check_one_token_int(&token_list,123);
	TEST_ASSERT(!glsl_lexer_extract_tokens("987U",&token_list));
	_check_one_token_int(&token_list,987);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("1ux",&token_list),"Unexpected character 'x'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("9Uu",&token_list),"Unexpected character 'u'"));
	TEST_GROUP("floats");
	TEST_ASSERT(!glsl_lexer_extract_tokens("123.456",&token_list));
	_check_one_token_float(&token_list,123.456);
	TEST_ASSERT(!glsl_lexer_extract_tokens(".999",&token_list));
	_check_one_token_float(&token_list,.999);
	TEST_ASSERT(!glsl_lexer_extract_tokens("2.",&token_list));
	_check_one_token_float(&token_list,2.0);
	TEST_ASSERT(!glsl_lexer_extract_tokens("123.456e2",&token_list));
	_check_one_token_float(&token_list,12345.6);
	TEST_ASSERT(!glsl_lexer_extract_tokens("314E-2",&token_list));
	_check_one_token_float(&token_list,3.14);
	TEST_ASSERT(!glsl_lexer_extract_tokens(".2e+1",&token_list));
	_check_one_token_float(&token_list,2.0);
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("1.2x",&token_list),"Decimal digit expected, got 'x'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("1.2E-3y",&token_list),"Decimal digit expected, got 'y'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("1E+2z",&token_list),"Decimal digit expected, got 'z'"));
	TEST_GROUP("unexpected character");
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("$",&token_list),"Unexpected character '$'"));
	TEST_ASSERT(test_glsl_check_and_cleanup_error(glsl_lexer_extract_tokens("123#",&token_list),"Unexpected character '#'"));
}
