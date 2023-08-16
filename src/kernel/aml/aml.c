#include <kernel/log/log.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/kernel.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "aml"



#define OPCODE_ZERO 0x00
#define OPCODE_ONE 0x01
#define OPCODE_ALIAS 0x06
#define OPCODE_NAME 0x08
#define OPCODE_BYTE_PREFIX 0x0a
#define OPCODE_WORD_PREFIX 0x0b
#define OPCODE_DWORD_PREFIX 0x0c
#define OPCODE_STRING_PREFIX 0x0d
#define OPCODE_QWORD_PREFIX 0x0e
#define OPCODE_SCOPE 0x10
#define OPCODE_BUFFER 0x11
#define OPCODE_PACKAGE 0x12
#define OPCODE_VAR_PACKAGE 0x13
#define OPCODE_METHOD 0x14
#define OPCODE_LOCAL0 0x60
#define OPCODE_LOCAL1 0x61
#define OPCODE_LOCAL2 0x62
#define OPCODE_LOCAL3 0x63
#define OPCODE_LOCAL4 0x64
#define OPCODE_LOCAL5 0x65
#define OPCODE_LOCAL6 0x66
#define OPCODE_LOCAL7 0x67
#define OPCODE_ARG0 0x68
#define OPCODE_ARG1 0x69
#define OPCODE_ARG2 0x6a
#define OPCODE_ARG3 0x6b
#define OPCODE_ARG4 0x6c
#define OPCODE_ARG5 0x6d
#define OPCODE_ARG6 0x6e
#define OPCODE_STORE 0x70
#define OPCODE_REF_OF 0x71
#define OPCODE_ADD 0x72
#define OPCODE_CONCAT 0x73
#define OPCODE_SUBTRACT 0x74
#define OPCODE_INCREMENT 0x75
#define OPCODE_DECREMENT 0x76
#define OPCODE_MULTIPLY 0x77
#define OPCODE_DIVIDE 0x78
#define OPCODE_SHIFT_LEFT 0x79
#define OPCODE_SHIFT_RIGHT 0x7a
#define OPCODE_AND 0x7b
#define OPCODE_NAND 0x7c
#define OPCODE_OR 0x7d
#define OPCODE_NOR 0x7e
#define OPCODE_XOR 0x7f
#define OPCODE_NOT 0x80
#define OPCODE_FIND_SET_LEFT_BIT 0x81
#define OPCODE_FIND_SET_RIGHT_BIT 0x82
#define OPCODE_DEREF_OF 0x83
#define OPCODE_CONCAT_RES 0x84
#define OPCODE_MOD 0x85
#define OPCODE_NOTIFY 0x86
#define OPCODE_SIZE_OF 0x87
#define OPCODE_INDEX 0x88
#define OPCODE_MATCH 0x89
#define OPCODE_CREATE_D_WORD_FIELD 0x8a
#define OPCODE_CREATE_WORD_FIELD 0x8b
#define OPCODE_CREATE_BYTE_FIELD 0x8c
#define OPCODE_CREATE_BIT_FIELD 0x8d
#define OPCODE_OBJECT_TYPE 0x8e
#define OPCODE_CREATE_Q_WORD_FIELD 0x8f
#define OPCODE_L_AND 0x90
#define OPCODE_L_OR 0x91
#define OPCODE_L_NOT 0x92
#define OPCODE_L_EQUAL 0x93
#define OPCODE_L_GREATER 0x94
#define OPCODE_L_LESS 0x95
#define OPCODE_TO_BUFFER 0x96
#define OPCODE_TO_DECIMAL_STRING 0x97
#define OPCODE_TO_HEX_STRING 0x98
#define OPCODE_TO_INTEGER 0x99
#define OPCODE_TO_STRING 0x9c
#define OPCODE_COPY_OBJECT 0x9d
#define OPCODE_MID 0x9e
#define OPCODE_CONTINUE 0x9f
#define OPCODE_IF 0xa0
#define OPCODE_ELSE 0xa1
#define OPCODE_WHILE 0xa2
#define OPCODE_NOOP 0xa3
#define OPCODE_RETURN 0xa4
#define OPCODE_BREAK 0xa5
#define OPCODE_BREAK_POINT 0xcc
#define OPCODE_ONES 0xff

#define OPCODE_EXT_MUTEX 0x01
#define OPCODE_EXT_EVENT 0x02
#define OPCODE_EXT_COND_REF_OF 0x12
#define OPCODE_EXT_CREATE_FIELD 0x13
#define OPCODE_EXT_LOAD_TABLE 0x1f
#define OPCODE_EXT_LOAD 0x20
#define OPCODE_EXT_STALL 0x21
#define OPCODE_EXT_SLEEP 0x22
#define OPCODE_EXT_ACQUIRE 0x23
#define OPCODE_EXT_SIGNAL 0x24
#define OPCODE_EXT_WAIT 0x25
#define OPCODE_EXT_RESET 0x26
#define OPCODE_EXT_RELEASE 0x27
#define OPCODE_EXT_FROM_BCD 0x28
#define OPCODE_EXT_TO_BCD 0x29
#define OPCODE_EXT_UNLOAD 0x2a
#define OPCODE_EXT_REVISION 0x30
#define OPCODE_EXT_DEBUG 0x31
#define OPCODE_EXT_FATAL 0x32
#define OPCODE_EXT_TIMER 0x33
#define OPCODE_EXT_REGION 0x80
#define OPCODE_EXT_FIELD 0x81
#define OPCODE_EXT_DEVICE 0x82
#define OPCODE_EXT_PROCESSOR 0x83
#define OPCODE_EXT_POWER_RES 0x84
#define OPCODE_EXT_THERMAL_ZONE 0x85
#define OPCODE_EXT_INDEX_FIELD 0x86
#define OPCODE_EXT_BANK_FIELD 0x87
#define OPCODE_EXT_DATA_REGION 0x88

#define OPCODE_FLAG_EXTENDED 0x01
#define OPCODE_FLAG_PKGLENGTH 0x02
#define OPCODE_FLAG_EXTRA_BYTES 0x04

#define OPCODE_ARG_UINT8 1
#define OPCODE_ARG_UINT16 2
#define OPCODE_ARG_UINT32 3
#define OPCODE_ARG_UINT64 4
#define OPCODE_ARG_STRING 5
#define OPCODE_ARG_NAME 6
#define OPCODE_ARG_OBJECT 7



#define _DECL_OPCODE_ARG_COUNT(_,_0,_1,_2,_3,_4,_5,n,...) n
#define DECL_OPCODE(opcode,flags,...) {opcode,flags,_DECL_OPCODE_ARG_COUNT(_,##__VA_ARGS__,6,5,4,3,2,1,0),{__VA_ARGS__}}



typedef struct _AML_OPCODE{
	u16 opcode;
	u8 flags;
	u8 arg_count;
	u8 args[6];
} aml_opcode_t;



static const aml_opcode_t _aml_opcodes[]={
	DECL_OPCODE(OPCODE_ZERO,0),
	DECL_OPCODE(OPCODE_ONE,0),
	DECL_OPCODE(OPCODE_ALIAS,0,OPCODE_ARG_NAME,OPCODE_ARG_NAME),
	DECL_OPCODE(OPCODE_NAME,0,OPCODE_ARG_NAME,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_BYTE_PREFIX,0,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_WORD_PREFIX,0,OPCODE_ARG_UINT16),
	DECL_OPCODE(OPCODE_DWORD_PREFIX,0,OPCODE_ARG_UINT32),
	DECL_OPCODE(OPCODE_STRING_PREFIX,0,OPCODE_ARG_STRING),
	DECL_OPCODE(OPCODE_QWORD_PREFIX,0,OPCODE_ARG_UINT64),
	DECL_OPCODE(OPCODE_SCOPE,OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_NAME),
	DECL_OPCODE(OPCODE_BUFFER,OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_PACKAGE,OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_VAR_PACKAGE,OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_METHOD,OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_NAME,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_LOCAL0,0),
	DECL_OPCODE(OPCODE_LOCAL1,0),
	DECL_OPCODE(OPCODE_LOCAL2,0),
	DECL_OPCODE(OPCODE_LOCAL3,0),
	DECL_OPCODE(OPCODE_LOCAL4,0),
	DECL_OPCODE(OPCODE_LOCAL5,0),
	DECL_OPCODE(OPCODE_LOCAL6,0),
	DECL_OPCODE(OPCODE_LOCAL7,0),
	DECL_OPCODE(OPCODE_ARG0,0),
	DECL_OPCODE(OPCODE_ARG1,0),
	DECL_OPCODE(OPCODE_ARG2,0),
	DECL_OPCODE(OPCODE_ARG3,0),
	DECL_OPCODE(OPCODE_ARG4,0),
	DECL_OPCODE(OPCODE_ARG5,0),
	DECL_OPCODE(OPCODE_ARG6,0),
	DECL_OPCODE(OPCODE_STORE,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_REF_OF,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_ADD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_CONCAT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_SUBTRACT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_INCREMENT,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_DECREMENT,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_MULTIPLY,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_DIVIDE,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_SHIFT_LEFT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_SHIFT_RIGHT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_AND,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_NAND,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_OR,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_NOR,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_XOR,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_NOT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_FIND_SET_LEFT_BIT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_FIND_SET_RIGHT_BIT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_DEREF_OF,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_CONCAT_RES,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_MOD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_NOTIFY,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_SIZE_OF,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_INDEX,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_MATCH,0,OPCODE_ARG_OBJECT,OPCODE_ARG_UINT8,OPCODE_ARG_OBJECT,OPCODE_ARG_UINT8,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_CREATE_D_WORD_FIELD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_NAME,  ),
	DECL_OPCODE(OPCODE_CREATE_WORD_FIELD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_NAME,  ),
	DECL_OPCODE(OPCODE_CREATE_BYTE_FIELD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_NAME,  ),
	DECL_OPCODE(OPCODE_CREATE_BIT_FIELD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_NAME,  ),
	DECL_OPCODE(OPCODE_OBJECT_TYPE,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_CREATE_Q_WORD_FIELD,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_NAME,  ),
	DECL_OPCODE(OPCODE_L_AND,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_L_OR,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_L_NOT,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_L_EQUAL,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_L_GREATER,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_L_LESS,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_TO_BUFFER,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_TO_DECIMAL_STRING,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_TO_HEX_STRING,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_TO_INTEGER,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_TO_STRING,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_COPY_OBJECT,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_MID,0,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_CONTINUE,0),
	DECL_OPCODE(OPCODE_IF,OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_ELSE,OPCODE_FLAG_PKGLENGTH),
	DECL_OPCODE(OPCODE_WHILE,OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_NOOP,0),
	DECL_OPCODE(OPCODE_RETURN,0,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_BREAK,0),
	DECL_OPCODE(OPCODE_BREAK_POINT,0),
	DECL_OPCODE(OPCODE_ONES,0),
	DECL_OPCODE(OPCODE_EXT_MUTEX,OPCODE_FLAG_EXTENDED,OPCODE_ARG_NAME,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_EXT_EVENT,OPCODE_FLAG_EXTENDED,OPCODE_ARG_NAME),
	DECL_OPCODE(OPCODE_EXT_COND_REF_OF,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_CREATE_FIELD,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_NAME),
	DECL_OPCODE(OPCODE_EXT_LOAD_TABLE,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_LOAD,OPCODE_FLAG_EXTENDED,OPCODE_ARG_NAME,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_STALL,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_SLEEP,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_ACQUIRE,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_UINT16),
	DECL_OPCODE(OPCODE_EXT_SIGNAL,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_WAIT,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_RESET,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_RELEASE,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_FROM_BCD,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_TO_BCD,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_UNLOAD,OPCODE_FLAG_EXTENDED,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_REVISION,OPCODE_FLAG_EXTENDED),
	DECL_OPCODE(OPCODE_EXT_DEBUG,OPCODE_FLAG_EXTENDED),
	DECL_OPCODE(OPCODE_EXT_FATAL,OPCODE_FLAG_EXTENDED,OPCODE_ARG_UINT8,OPCODE_ARG_UINT32,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_TIMER,OPCODE_FLAG_EXTENDED),
	DECL_OPCODE(OPCODE_EXT_REGION,OPCODE_FLAG_EXTENDED,OPCODE_ARG_NAME,OPCODE_ARG_UINT8,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	DECL_OPCODE(OPCODE_EXT_FIELD,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,OPCODE_ARG_NAME,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_EXT_DEVICE,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_NAME),
	DECL_OPCODE(OPCODE_EXT_PROCESSOR,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_NAME,OPCODE_ARG_UINT8,OPCODE_ARG_UINT32,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_EXT_POWER_RES,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_NAME,OPCODE_ARG_UINT8,OPCODE_ARG_UINT16),
	DECL_OPCODE(OPCODE_EXT_THERMAL_ZONE,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH,OPCODE_ARG_NAME),
	DECL_OPCODE(OPCODE_EXT_INDEX_FIELD,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,OPCODE_ARG_NAME,OPCODE_ARG_NAME,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_EXT_BANK_FIELD,OPCODE_FLAG_EXTENDED|OPCODE_FLAG_PKGLENGTH|OPCODE_FLAG_EXTRA_BYTES,OPCODE_ARG_NAME,OPCODE_ARG_NAME,OPCODE_ARG_OBJECT,OPCODE_ARG_UINT8),
	DECL_OPCODE(OPCODE_EXT_DATA_REGION,OPCODE_FLAG_EXTENDED,OPCODE_ARG_NAME,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT,OPCODE_ARG_OBJECT),
	{0xff,0xff}
};



typedef struct _OBJECT{
	u8 opcode;
	u32 data_length;
	union{
		u64 u64;
		const char* str;
		struct{
			const char* data;
			u8 length;
		} name;
		struct _OBJECT* object;
	} args[6];
	union{
		const u8* bytes;
		struct _OBJECT* objects;
	} data;
} object_t;



typedef struct _ALLOCATOR{
	u64 top;
	u64 max_top;
} allocator_t;



typedef struct _OBJECT_TARGET{
	object_t* object;
	u8 arg_index;
} object_target_t;



// static void* _allocate_data(allocator_t* allocator,u32 size){
// 	size=(size+7)&0xfffffffffffffff8ull;
// 	while (allocator->top+size>allocator->max_top){
// 		vmm_map_page(&vmm_kernel_pagemap,pmm_alloc(1,PMM_COUNTER_CPU),allocator->max_top,VMM_PAGE_FLAG_PRESENT|VMM_PAGE_FLAG_READWRITE|VMM_PAGE_FLAG_USER);
// 		allocator->max_top+=PAGE_SIZE;
// 	}
// 	void* out=(void*)(allocator->top);
// 	allocator->top+=size;
// 	return out;
// }



// static void* _allocate_object(allocator_t* allocator,u8 type,u32 size,u32 array_size){
// 	object_header_t* out=_allocate_data(allocator,size);
// 	out->type=type;
// 	if (array_size!=OBJECT_NO_ARRAY_SIZE){
// 		object_array_header_t* array=(object_array_header_t*)out;
// 		array->length=array_size;
// 		array->data=_allocate_data(allocator,array_size*sizeof(object_header_t*));
// 	}
// 	return out;
// }



static u32 _get_name_length(const u8* data){
	u32 out=0;
	if (data[out]=='\\'){
		out++;
	}
	while (data[out]=='^'){
		out++;
	}
	u8 segment_count=1;
	if (data[out]=='.'){
		out++;
		segment_count=2;
	}
	else if (data[out]=='/'){
		out++;
		segment_count=data[out];
		out++;
	}
	else if (!data[out]){
		out++;
		segment_count=0;
	}
	return out+(segment_count<<2);
}



static const aml_opcode_t* _parse_opcode(const u8* data){
	u8 extended=0;
	u8 type=data[0];
	if (type==0x5b){
		extended=OPCODE_FLAG_EXTENDED;
		type=data[1];
	}
	for (const aml_opcode_t* out=_aml_opcodes;out->flags!=0xff;out++){
		if ((out->flags&OPCODE_FLAG_EXTENDED)==extended&&out->opcode==type){
			return out;
		}
	}
	ERROR("Unknown opcode '%s%x'",(extended?"5b ":""),type);
	WARN("%x %x %x %x",data[0],data[1],data[2],data[3]);
	for (;;);
}



static u32 _get_opcode_encoding_length(const aml_opcode_t* opcode){
	return ((opcode->flags&OPCODE_FLAG_EXTENDED)?2:1);
}



static u32 _get_pkglength_encoding_length(const u8* data){
	return 1+(data[0]>>6);
}



static u32 _get_pkglength(const u8* data){
	u32 out=data[0]&0x3f;
	for (u8 i=0;i<(data[0]>>6);i++){
		out|=data[i+1]<<(4+8*i);
	}
	return out;
}



static u32 _get_opcode_size(const u8* data,const aml_opcode_t* opcode){
	u32 out=_get_opcode_encoding_length(opcode);
	if (opcode->flags&OPCODE_FLAG_PKGLENGTH){
		return out+_get_pkglength(data+out);
	}
	for (u8 i=0;i<opcode->arg_count;i++){
		if (opcode->args[i]==OPCODE_ARG_UINT8){
			out++;
		}
		else if (opcode->args[i]==OPCODE_ARG_UINT16){
			out+=2;
		}
		else if (opcode->args[i]==OPCODE_ARG_UINT32){
			out+=4;
		}
		else if (opcode->args[i]==OPCODE_ARG_UINT64){
			out+=8;
		}
		else if (opcode->args[i]==OPCODE_ARG_STRING){
			do{
				out++;
			} while (data[out-1]);
		}
		else if (opcode->args[i]==OPCODE_ARG_NAME){
			out+=_get_name_length(data+out);
		}
		else if (opcode->args[i]==OPCODE_ARG_OBJECT){
			out+=_get_opcode_size(data+out,_parse_opcode(data+out));
			ERROR("OPCODE_ARG_OBJECT");
		}
	}
	return out;
}



static u32 _parse_object(const u8* data,allocator_t* allocator,object_target_t* target){
	const u8* data_start=data;
	const u8* data_end=NULL;
	const aml_opcode_t* opcode=_parse_opcode(data);
	data+=_get_opcode_encoding_length(opcode);
	if (opcode->flags&OPCODE_FLAG_PKGLENGTH){
		data_end=data+_get_pkglength(data);
		data+=_get_pkglength_encoding_length(data);
	}
	if (target->arg_index!=0xff&&!(opcode->flags&OPCODE_FLAG_EXTENDED)){
		switch (opcode->opcode){
			case OPCODE_ZERO:
				target->object->args[target->arg_index].u64=0;
				return data-data_start;
			case OPCODE_ONE:
				target->object->args[target->arg_index].u64=1;
				return data-data_start;
			case OPCODE_BYTE_PREFIX:
				target->object->args[target->arg_index].u64=data[0];
				return data-data_start+1;
			case OPCODE_WORD_PREFIX:
				ERROR("OPCODE_WORD_PREFIX");for (;;);
				return data-data_start+2;
			case OPCODE_DWORD_PREFIX:
				ERROR("OPCODE_DWORD_PREFIX");for (;;);
				return data-data_start+4;
			case OPCODE_STRING_PREFIX:
				ERROR("OPCODE_STRING_PREFIX");for (;;);
				return data-data_start;
			case OPCODE_QWORD_PREFIX:
				ERROR("OPCODE_QWORD_PREFIX");for (;;);
				return data-data_start+8;
			case OPCODE_ONES:
				target->object->args[target->arg_index].u64=0xffffffffffffffffull;
				return data-data_start;
		}
	}
	WARN("~ %x",opcode->opcode);
	return _get_opcode_size(data_start,opcode);
	return data_end-data_start;
}



void aml_load(const u8* data,u32 length){
	LOG("Loading AML...");
	allocator_t allocator={
		pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset()),
		pmm_align_up_address(kernel_get_bss_end()+kernel_get_offset())
	};
	for (u32 offset=0;offset<length;){
		object_t tmp;
		object_target_t target={
			&tmp,
			0xff
		};
		offset+=_parse_object(data+offset,&allocator,&target);
	}
	for (;;);
}
