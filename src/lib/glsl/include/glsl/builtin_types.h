#ifndef _GLSL_BUILTIN_TYPES_H_
#define _GLSL_BUILTIN_TYPES_H_ 1
#include <sys/types.h>



#define GLSL_BUILTIN_TYPE_NONE 0
#define GLSL_BUILTIN_TYPE_INT 1
#define GLSL_BUILTIN_TYPE_VOID 2
#define GLSL_BUILTIN_TYPE_BOOL 3
#define GLSL_BUILTIN_TYPE_FLOAT 4
#define GLSL_BUILTIN_TYPE_MAT22 5
#define GLSL_BUILTIN_TYPE_MAT33 6
#define GLSL_BUILTIN_TYPE_MAT44 7
#define GLSL_BUILTIN_TYPE_MAT23 8
#define GLSL_BUILTIN_TYPE_MAT24 9
#define GLSL_BUILTIN_TYPE_MAT32 10
#define GLSL_BUILTIN_TYPE_MAT34 11
#define GLSL_BUILTIN_TYPE_MAT42 12
#define GLSL_BUILTIN_TYPE_MAT43 13
#define GLSL_BUILTIN_TYPE_VEC2 14
#define GLSL_BUILTIN_TYPE_VEC3 15
#define GLSL_BUILTIN_TYPE_VEC4 16
#define GLSL_BUILTIN_TYPE_IVEC2 17
#define GLSL_BUILTIN_TYPE_IVEC3 18
#define GLSL_BUILTIN_TYPE_IVEC4 19
#define GLSL_BUILTIN_TYPE_BVEC2 20
#define GLSL_BUILTIN_TYPE_BVEC3 21
#define GLSL_BUILTIN_TYPE_BVEC4 22
#define GLSL_BUILTIN_TYPE_UINT 23
#define GLSL_BUILTIN_TYPE_UVEC2 24
#define GLSL_BUILTIN_TYPE_UVEC3 25
#define GLSL_BUILTIN_TYPE_UVEC4 26
#define GLSL_BUILTIN_TYPE_SAMPLER_1D 27
#define GLSL_BUILTIN_TYPE_SAMPLER_2D 28
#define GLSL_BUILTIN_TYPE_SAMPLER_3D 29
#define GLSL_BUILTIN_TYPE_SAMPLER_CB 30
#define GLSL_BUILTIN_TYPE_SAMPLER_1D_SHADOW 31
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_SHADOW 32
#define GLSL_BUILTIN_TYPE_SAMPLER_CB_SHADOW 33
#define GLSL_BUILTIN_TYPE_SAMPLER_1D_ARRAY 34
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_ARRAY 35
#define GLSL_BUILTIN_TYPE_SAMPLER_1D_ARRAY_SHADOW 36
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_ARRAY_SHADOW 37
#define GLSL_BUILTIN_TYPE_ISAMPLER_1D 38
#define GLSL_BUILTIN_TYPE_ISAMPLER_2D 39
#define GLSL_BUILTIN_TYPE_ISAMPLER_3D 40
#define GLSL_BUILTIN_TYPE_ISAMPLER_CB 41
#define GLSL_BUILTIN_TYPE_ISAMPLER_1D_ARRAY 42
#define GLSL_BUILTIN_TYPE_ISAMPLER_2D_ARRAY 43
#define GLSL_BUILTIN_TYPE_USAMPLER_1D 44
#define GLSL_BUILTIN_TYPE_USAMPLER_2D 45
#define GLSL_BUILTIN_TYPE_USAMPLER_3D 46
#define GLSL_BUILTIN_TYPE_USAMPLER_CB 47
#define GLSL_BUILTIN_TYPE_USAMPLER_1D_ARRAY 48
#define GLSL_BUILTIN_TYPE_USAMPLER_2D_ARRAY 49
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_RECT 50
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_RECT_SHADOW 51
#define GLSL_BUILTIN_TYPE_ISAMPLER_2D_RECT 52
#define GLSL_BUILTIN_TYPE_USAMPLER_2D_RECT 53
#define GLSL_BUILTIN_TYPE_SAMPLER_BUFFER 54
#define GLSL_BUILTIN_TYPE_ISAMPLER_BUFFER 55
#define GLSL_BUILTIN_TYPE_USAMPLER_BUFFER 56
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_MULTI_SAMPLE 57
#define GLSL_BUILTIN_TYPE_ISAMPLER_2D_MULTI_SAMPLE 58
#define GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE 59
#define GLSL_BUILTIN_TYPE_SAMPLER_2D_MULTI_SAMPLE_ARRAY 60
#define GLSL_BUILTIN_TYPE_ISAMPLER_2D_MULTI_SAMPLE_ARRAY 61
#define GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE_ARRAY 62

#define GLSL_BUILTIN_MAX_TYPE GLSL_BUILTIN_TYPE_USAMPLER_2D_MULTI_SAMPLE_ARRAY



typedef u32 glsl_builtin_type_t;



_Bool glsl_builtin_type_is_compatible(glsl_builtin_type_t src,glsl_builtin_type_t dst);



const char* glsl_builtin_type_to_string(glsl_builtin_type_t builtin_type);



u32 glsl_builtin_type_to_size(glsl_builtin_type_t builtin_type);



u32 glsl_builtin_type_to_slot_count(glsl_builtin_type_t builtin_type);



u32 glsl_builtin_type_to_vector_length(glsl_builtin_type_t builtin_type);



u32 glsl_builtin_type_to_vector_base_type(glsl_builtin_type_t builtin_type);



u32 glsl_builtin_type_from_base_type_and_length(glsl_builtin_type_t builtin_type,u32 length);



#endif
