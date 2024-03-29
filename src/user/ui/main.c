#include <GL/gl.h>
#include <opengl/opengl.h>
#include <sys/clock/clock.h>
#include <sys/error/error.h>
#include <sys/heap/heap.h>
#include <sys/io/io.h>
#include <sys/mp/thread.h>
#include <sys/mp/timer.h>
#include <sys/types.h>
#include <ui/display.h>



static const char*const _ui_framebuffer_format_names[UI_DISPLAY_FRAMEBUFFER_FORMAT_MAX+1]={
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_BGRX]="BGRX",
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_RGBX]="RGBX",
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_XBGR]="XBGR",
	[UI_DISPLAY_FRAMEBUFFER_FORMAT_XRGB]="XRGB",
};

static const char* _bg_vertex_shader=" \
#version 330 core \n\
 \n\
 \n\
 \n\
layout (location=0) in vec2 in_pos; \n\
layout (location=1) in vec2 in_uv; \n\
out vec2 fs_uv; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	gl_Position=vec4(in_pos,0.0,1.0); \n\
	fs_uv=in_uv; \n\
} \n\
";

static const char* _bg_fragment_shader=" \
#version 330 core \n\
 \n\
 \n\
 \n\
in vec2 fs_uv; \n\
uniform sampler2D fs_texture; \n\
out vec4 out_color; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	out_color=vec4(__gl_texture(fs_texture,fs_uv).rgb,1.0); \n\
} \n\
";

static const char* _vertex_shader=" \
#version 330 core \n\
 \n\
 \n\
 \n\
layout (location=0) in vec2 in_pos; \n\
layout (location=1) in vec3 in_color; \n\
layout (location=2) in vec2 in_uv; \n\
uniform vec4 vs_color; \n\
uniform mat3x3 vs_transform; \n\
out vec4 fs_color; \n\
out vec2 fs_uv; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	vec3 local=vs_transform*vec3(in_pos,1.0); \n\
	gl_Position=vec4(local.xy,0.0,1.0); \n\
	fs_color=vs_color/2.0+vec4(in_color,1.0); \n\
	fs_uv=in_uv; \n\
} \n\
";

static const char* _fragment_shader=" \
#version 330 core \n\
 \n\
 \n\
 \n\
in vec4 fs_color; \n\
in vec2 fs_uv; \n\
uniform sampler2D fs_texture; \n\
out vec4 out_color; \n\
 \n\
 \n\
 \n\
void main(void){ \n\
	out_color=fs_color*0.45+vec4(__gl_texture(fs_texture,fs_uv).rgb,1.0); \n\
} \n\
";



static void _hsl_to_rgb(u8 h,u8 s,u8 l,u8* rgb){
	if (!s){
		rgb[0]=l;
		rgb[1]=l;
		rgb[2]=l;
		return;
	}
	u8 r=h/43;
	u8 m=(h-(r*43))*6;
	u8 p=(l*(255-s))>>8;
	u8 q=(l*(255-((s*m)>>8)))>>8;
	u8 t=(l*(255-((s*(255-m))>>8)))>>8;
	switch (r){
		case 0:
			rgb[0]=l;
			rgb[1]=t;
			rgb[2]=p;
			break;
		case 1:
			rgb[0]=q;
			rgb[1]=l;
			rgb[2]=p;
			break;
		case 2:
			rgb[0]=p;
			rgb[1]=l;
			rgb[2]=t;
			break;
		case 3:
			rgb[0]=p;
			rgb[1]=q;
			rgb[2]=l;
			break;
		case 4:
			rgb[0]=t;
			rgb[1]=p;
			rgb[2]=l;
			break;
		default:
			rgb[0]=l;
			rgb[1]=p;
			rgb[2]=q;
			break;
	}
}



extern void ui_permission_thread_start(void);



int main(int argc,const char** argv){
	ui_permission_thread_start();
	opengl_init();
	for (ui_display_handle_t display=ui_display_iter_start();display;display=ui_display_iter_next(display)){
		ui_display_data_t data;
		if (SYS_IS_ERROR(ui_display_get_data(display,&data))){
			continue;
		}
		sys_io_print("Display #%u: %u x %u @ %u Hz\n",data.index,data.mode.width,data.mode.height,data.mode.freq);
		ui_display_info_t temp_info;
		if (SYS_IS_ERROR(ui_display_get_info(display,&temp_info,sizeof(ui_display_info_t)))){
			continue;
		}
		ui_display_info_t* info=sys_heap_alloc(NULL,sizeof(ui_display_info_t)+temp_info.mode_count*sizeof(ui_display_mode_t));
		if (SYS_IS_ERROR(ui_display_get_info(display,info,sizeof(ui_display_info_t)+temp_info.mode_count*sizeof(ui_display_mode_t)))){
			continue;
		}
		sys_io_print("Modes: (%u)\n",temp_info.mode_count);
		for (u32 i=0;i<temp_info.mode_count;i++){
			sys_io_print("  %u x %u @ %u Hz\n",info->modes[i].width,info->modes[i].height,info->modes[i].freq);
		}
		sys_heap_dealloc(NULL,info);
		ui_framebuffer_handle_t framebuffer=ui_display_get_display_framebuffer(display);
		ui_display_framebuffer_t config;
		ui_display_get_framebuffer_config(framebuffer,&config);
		sys_io_print("Framebuffer: %u x %u, %s\n",config.width,config.height,_ui_framebuffer_format_names[config.format]);
		opengl_state_t state=opengl_create_state(330);
		opengl_set_state_framebuffer(state,framebuffer);
		opengl_set_state(state);
		sys_io_print("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
		sys_io_print("GL_SHADING_LANGUAGE_VERSION: %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
		sys_io_print("GL_VENDOR: %s\n",glGetString(GL_VENDOR));
		sys_io_print("GL_VERSION: %s\n",glGetString(GL_VERSION));
		GLint extension_count;
		glGetIntegerv(GL_NUM_EXTENSIONS,&extension_count);
		sys_io_print("GL_NUM_EXTENSIONS: %u\n",extension_count);
		GLint uniform_vs_color;
		GLuint bg_program=glCreateProgram();
		{
			GLuint vertex_shader=glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader,1,&_bg_vertex_shader,NULL);
			glCompileShader(vertex_shader);
			GLint compilation_status;
			glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&compilation_status);
			if (!compilation_status){
				GLsizei length;
				glGetShaderiv(vertex_shader,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetShaderInfoLog(vertex_shader,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			GLuint fragment_shader=glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader,1,&_bg_fragment_shader,NULL);
			glCompileShader(fragment_shader);
			glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&compilation_status);
			if (!compilation_status){
				GLsizei length;
				glGetShaderiv(fragment_shader,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetShaderInfoLog(fragment_shader,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			glAttachShader(bg_program,vertex_shader);
			glAttachShader(bg_program,fragment_shader);
			glLinkProgram(bg_program);
			GLint link_status;
			glGetProgramiv(bg_program,GL_LINK_STATUS,&link_status);
			if (!link_status){
				GLsizei length;
				glGetProgramiv(bg_program,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetProgramInfoLog(bg_program,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			glUseProgram(bg_program);
			glUniform1i(glGetUniformLocation(bg_program,"fs_texture"),0);
		}
		GLuint program=glCreateProgram();
		{
			GLuint vertex_shader=glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader,1,&_vertex_shader,NULL);
			glCompileShader(vertex_shader);
			GLint compilation_status;
			glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&compilation_status);
			if (!compilation_status){
				GLsizei length;
				glGetShaderiv(vertex_shader,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetShaderInfoLog(vertex_shader,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			GLuint fragment_shader=glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader,1,&_fragment_shader,NULL);
			glCompileShader(fragment_shader);
			glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&compilation_status);
			if (!compilation_status){
				GLsizei length;
				glGetShaderiv(fragment_shader,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetShaderInfoLog(fragment_shader,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			glAttachShader(program,vertex_shader);
			glAttachShader(program,fragment_shader);
			glLinkProgram(program);
			GLint link_status;
			glGetProgramiv(program,GL_LINK_STATUS,&link_status);
			if (!link_status){
				GLsizei length;
				glGetProgramiv(program,GL_INFO_LOG_LENGTH,&length);
				char* buffer=sys_heap_alloc(NULL,length);
				glGetProgramInfoLog(program,length,&length,buffer);
				sys_io_print("%s\n",buffer);
				sys_heap_dealloc(NULL,buffer);
				return 1;
			}
			glUseProgram(program);
			uniform_vs_color=glGetUniformLocation(program,"vs_color");
			GLint uniform_vs_transform=glGetUniformLocation(program,"vs_transform");
			GLint uniform_fs_texture=glGetUniformLocation(program,"fs_texture");
			sys_io_print("uniform.vs_color=%u\nuniform.vs_transform=%u\nuniform.fs_texture=%u\n",uniform_vs_color,uniform_vs_transform,uniform_fs_texture);
			float matrix[9]={1.0f,0.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,0.0f,1.0f};
			glUniformMatrix3fv(uniform_vs_transform,1,GL_FALSE,matrix);
			glUniform1i(uniform_fs_texture,0);
		}
		u64 timer_interval=1000000000ull/data.mode.freq;
		sys_timer_t timer=sys_timer_create(0,0);
		sys_event_t timer_event=sys_timer_get_event(timer);
		glViewport(0,0,config.width,config.height);
		GLuint bg_vao;
		GLuint bg_vbo[2];
		glGenVertexArrays(1,&bg_vao);
		glGenBuffers(2,bg_vbo);
		glBindVertexArray(bg_vao);
		glBindBuffer(GL_ARRAY_BUFFER,bg_vbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bg_vbo[1]);
		const float bg_buffer[]={
			-1.0f,+1.0f,0.0f,0.0f,
			+1.0f,+1.0f,1.0f,0.0f,
			+1.0f,-1.0f,1.0f,1.0f,
			-1.0f,-1.0f,0.0f,1.0f,
		};
		glBufferData(GL_ARRAY_BUFFER,sizeof(bg_buffer),bg_buffer,GL_STATIC_DRAW);
		const u16 bg_indices[6]={0,1,2,0,2,3};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(bg_indices),bg_indices,GL_STATIC_DRAW);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,4*sizeof(float),NULL);
		glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,4*sizeof(float),(void*)(2*sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		GLuint bg_texture_id;
		glGenTextures(1,&bg_texture_id);
		glBindTexture(GL_TEXTURE_2D,bg_texture_id);
		const u32 bg_texture_data[4*4]={
			0xff0000,0xff0000,0xff0000,0xff0000,
			0xff0000,0x00ff00,0x00ff00,0xff0000,
			0xff0000,0x00ff00,0x00ff00,0xff0000,
			0xff0000,0xff0000,0xff0000,0x0000ff,
		};
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,4,4,0,GL_RGBA,GL_UNSIGNED_BYTE,bg_texture_data);
		GLuint vao;
		GLuint vbo[2];
		glGenVertexArrays(1,&vao);
		glGenBuffers(2,vbo);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[1]);
		float buffer[]={
			+0.0f,+1.0f,1.0f,0.0f,0.0f,0.0f,0.0f,
			-1.0f,-1.0f,0.0f,1.0f,0.0f,1.0f,0.0f,
			+1.0f,-1.0f,0.0f,0.0f,1.0f,0.0f,1.0f,
		};
		glBufferData(GL_ARRAY_BUFFER,sizeof(buffer),buffer,GL_DYNAMIC_DRAW);
		const u32 indices[3]={0,1,2};
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
		glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,7*sizeof(float),NULL);
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,7*sizeof(float),(void*)(2*sizeof(float)));
		glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,7*sizeof(float),(void*)(5*sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttrib4f(1,1.0f,0.25f,0.5f,1.0f);
		GLuint texture_id;
		glGenTextures(1,&texture_id);
		glBindTexture(GL_TEXTURE_2D,texture_id);
		const u32 texture_data[2*3]={
			0xff0000,0x00ff00,0x00ffff,
			0x0000ff,0xffff00,0xff00ff
		};
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,2,3,0,GL_RGBA,GL_UNSIGNED_BYTE,texture_data);
		float start=sys_clock_get_time();
		for (u64 frame=0;;frame++){
			sys_timer_update(timer,timer_interval,1);
			if ((frame%data.mode.freq)==data.mode.freq-1){
				float end=sys_clock_get_time();
				if (0){
					sys_io_print("==> %f, %f fps\n",end-start,data.mode.freq/(end-start));
				}
				start=end;
			}
			u8 color[3];
			_hsl_to_rgb(frame*255/120,127,255,color);
			glClearColor(color[0]/255.0f,color[1]/255.0f,color[2]/255.0f,1.0f);
			_hsl_to_rgb((frame+60)*255/120,127,255,color);
			glUniform4f(uniform_vs_color,color[0]/255.0f,color[1]/255.0f,color[2]/255.0f,1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindVertexArray(bg_vao);
			glBindBuffer(GL_ARRAY_BUFFER,bg_vbo[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bg_vbo[1]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,bg_texture_id);
			glUseProgram(bg_program);
			glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_SHORT,NULL);
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER,vbo[0]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vbo[1]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,texture_id);
			glUseProgram(program);
			buffer[0]=(frame%120)/60.0f-1.0f;
			glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(float),buffer);
			glDrawArrays(GL_TRIANGLES,0,3);
			buffer[0]=1.0f-(frame%120)/60.0f;
			glBufferSubData(GL_ARRAY_BUFFER,0,sizeof(float),buffer);
			glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,NULL);
			glFlush();
			ui_display_flush_display_framebuffer(display);
			sys_thread_await_event(timer_event);
		}
	}
	return 0;
}
