#include <GL/gl.h>
#include <opengl/command_buffer.h>
#include <opengl/opengl.h>
#include <opengl/syscalls.h>
#include <sys/format/format.h>
#include <sys/io/io.h>
#include <sys/memory/memory.h>
#include <sys/syscall/syscall.h>
#include <sys/types.h>
#include <ui/display.h>



static _Bool _opengl_initialized=0;

opengl_state_id_t opengl_current_state_id=0;



SYS_PUBLIC void opengl_init(void){
	if (!opengl_syscalls_init()){
		sys_io_print("OpenGL module is not loaded\n");
		return;
	}
	opengl_command_buffer_init();
	_opengl_initialized=1;
}



SYS_PUBLIC opengl_state_t opengl_create_state(u16 min_version){
	if (!_opengl_initialized){
		return 0;
	}
	opengl_driver_instance_t driver_instance=opengl_syscall_get_driver_instance(min_version);
	opengl_driver_instance_data_t driver_instance_data;
	if (!driver_instance||!opengl_syscall_get_driver_instance_data(driver_instance,&driver_instance_data)){
		return 0;
	}
	opengl_internal_state_t* out=(void*)sys_memory_map(sys_memory_align_up_address(sizeof(opengl_internal_state_t)),SYS_MEMORY_FLAG_READ|SYS_MEMORY_FLAG_WRITE,0);
	out->state_id=opengl_syscall_create_state(driver_instance);
	out->driver_instance=driver_instance;
	out->driver_opengl_version=driver_instance_data.opengl_version;
	sys_format_string(out->gl_renderer,64,"%s",driver_instance_data.renderer_name);
	sys_format_string(out->gl_shading_language_version,16,"1.0");
	sys_format_string(out->gl_vendor,32,"opengl/%s",driver_instance_data.driver_name);
	sys_format_string(out->gl_version,16,"%u.%u.%u",out->driver_opengl_version/100,(out->driver_opengl_version/10)%10,out->driver_opengl_version%10);
	out->gl_error=GL_NO_ERROR;
	out->gl_active_texture=0;
	for (u8 i=0;i<4;i++){
		out->gl_clear_color_value[i]=0.0f;
	}
	out->gl_clear_depth_value=0.0f;
	out->gl_clear_stencil_value=0;
	for (u8 i=0;i<4;i++){
		out->gl_viewport[i]=0;
	}
	return out;
}



SYS_PUBLIC _Bool opengl_set_state(opengl_state_t state){
	if (!_opengl_initialized){
		return 0;
	}
	opengl_command_buffer_flush();
	opengl_internal_state_t* internal_state=state;
	opengl_current_state_id=internal_state->state_id;
	_gl_set_internal_state(internal_state);
	return 1;
}



SYS_PUBLIC _Bool opengl_set_state_framebuffer(opengl_state_t state,ui_framebuffer_handle_t framebuffer){
	if (!_opengl_initialized){
		return 0;
	}
	opengl_command_buffer_flush();
	opengl_internal_state_t* internal_state=state;
	return opengl_syscall_set_state_framebuffer(internal_state->state_id,framebuffer);
}
