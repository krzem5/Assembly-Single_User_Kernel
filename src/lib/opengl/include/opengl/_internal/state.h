#ifndef _OPENGL__INTERNAL_STATE_H_
#define _OPENGL__INTERNAL_STATE_H_ 1
#include <GL/gl.h>
#include <glsl/backend.h>
#include <glsl/compiler.h>
#include <glsl/linker.h>
#include <sys/types.h>



#define OPENGL_HANDLE_TYPE_NONE 0
#define OPENGL_HANDLE_TYPE_SHADER 1
#define OPENGL_HANDLE_TYPE_PROGRAM 2



typedef u32 opengl_handle_type_t;



typedef u64 opengl_driver_instance_t;



typedef u64 opengl_state_id_t;



typedef void* opengl_state_t;



typedef struct _OPENGL_SHADER_SOURCE{
	GLchar* data;
	GLuint64 length;
} opengl_shader_source_t;



typedef struct _OPENGL_HANDLE_HEADER{
	opengl_handle_type_t type;
	GLuint index;
} opengl_handle_header_t;



typedef struct _OPENGL_PROGRAM_STATE{
	opengl_handle_header_t header;
	glsl_linker_program_t linker_program;
	_Bool was_linkage_attempted;
	glsl_linker_linked_program_t linked_program;
	glsl_error_t error;
} opengl_program_state_t;



typedef struct _OPENGL_SHADER_STATE{
	opengl_handle_header_t header;
	GLenum type;
	GLuint source_count;
	opengl_shader_source_t* sources;
	_Bool was_compilation_attempted;
	glsl_compilation_output_t compilation_output;
	glsl_error_t error;
	opengl_program_state_t* program;
} opengl_shader_state_t;



typedef struct _OPENGL_DRIVER_INSTANCE_DATA{
	u16 opengl_version;
	char driver_name[32];
	char renderer_name[64];
	char library[128];
} opengl_driver_instance_data_t;



typedef struct _OPENGL_INTERNAL_STATE{
	opengl_state_id_t state_id;
	opengl_driver_instance_t driver_instance;
	u16 driver_opengl_version;
	const glsl_backend_descriptor_t* glsl_backend_descriptor;
	char gl_renderer[64];
	char gl_shading_language_version[16];
	char gl_vendor[32];
	char gl_version[16];
	GLenum gl_error;
	GLuint gl_active_texture;
	GLfloat gl_clear_color_value[4];
	GLdouble gl_clear_depth_value;
	GLint gl_clear_stencil_value;
	GLint gl_viewport[4];
	opengl_handle_header_t** handles;
	GLuint handle_count;
} opengl_internal_state_t;



extern opengl_state_id_t opengl_current_state_id;



#endif
