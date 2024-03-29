#ifndef _GLSL_BACKEND_H_
#define _GLSL_BACKEND_H_ 1
#include <glsl/compiler.h>
#include <glsl/error.h>
#include <glsl/linker.h>
#include <sys/types.h>



typedef struct _GLSL_BACKEND_DESCRIPTOR{
	const char* name;
	glsl_error_t (*shader_link_callback)(const glsl_compilation_output_t*,glsl_linker_linked_program_shader_t*);
} glsl_backend_descriptor_t;



typedef const glsl_backend_descriptor_t* (*glsl_backend_descriptor_query_func_t)(void);



#endif
