#include <opengl/command_buffer.h>
#include <opengl/protocol.h>
#include <opengl/syscalls.h>
#include <sys2/memory/memory.h>
#include <sys2/types.h>



static void* _opengl_command_buffer;
static u32 _opengl_command_buffer_size;



void opengl_command_buffer_init(void){
	_opengl_command_buffer=(void*)sys2_memory_map(OPENGL_COMMAND_BUFFER_SIZE,SYS2_MEMORY_FLAG_READ|SYS2_MEMORY_FLAG_WRITE,0);
	_opengl_command_buffer_size=0;
}



void opengl_command_buffer_set_lock(_Bool lock){
	// unimplemented
}



void opengl_command_buffer_ensure_space(u32 length){
	if (_opengl_command_buffer_size+length<=OPENGL_COMMAND_BUFFER_SIZE){
		return;
	}
	opengl_syscall_flush_command_buffer(_opengl_command_buffer,_opengl_command_buffer_size);
	_opengl_command_buffer_size=0;
}



const opengl_protocol_header_t* opengl_command_buffer_push(const opengl_protocol_header_t* header){
	opengl_protocol_header_t* out=_opengl_command_buffer+_opengl_command_buffer_size;
	for (u32 i=0;i<header->length/sizeof(u32);i++){
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wzero-length-bounds"
		out->_data[i]=header->_data[i];
#pragma GCC diagnostic pop
	}
	_opengl_command_buffer_size+=header->length;
	return out;
}



void opengl_command_buffer_push_single(const opengl_protocol_header_t* header){
	opengl_command_buffer_set_lock(1);
	opengl_command_buffer_ensure_space(header->length);
	opengl_command_buffer_push(header);
	opengl_command_buffer_set_lock(0);
}



void opengl_command_buffer_flush(void){
	if (!_opengl_command_buffer_size){
		return;
	}
	opengl_syscall_flush_command_buffer(_opengl_command_buffer,_opengl_command_buffer_size);
	_opengl_command_buffer_size=0;
}
