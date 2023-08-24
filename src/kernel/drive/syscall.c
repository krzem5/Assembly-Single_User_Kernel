#include <kernel/drive/drive.h>
#include <kernel/drive/drive_list.h>
#include <kernel/fs/kfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/partition/partition.h>
#include <kernel/syscall/syscall.h>
#include <kernel/types.h>
#include <kernel/util/util.h>



#define USER_DRIVE_FLAG_PRESENT 0
#define USER_DRIVE_FLAG_BOOT 1



typedef struct _USER_DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[16];
	char serial_number[32];
	char model_number[64];
	u64 block_count;
	u64 block_size;
} user_drive_t;



static drive_t* _get_drive(u64 index){
	for (drive_t* drive=drive_data;drive;drive=drive->next){
		if (drive->index==index){
			return drive;
		}
	}
	return NULL;
}



void syscall_drive_format(syscall_registers_t* regs){
	u64 address=0;
	if (regs->rdx){
		address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
		if (!address){
			regs->rax=0;
			return;
		}
	}
	const drive_t* drive=_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	regs->rax=kfs_format_drive(drive,(void*)address,regs->rdx);
}



void syscall_drive_stats(syscall_registers_t* regs){
	if (regs->rdx!=sizeof(drive_stats_t)){
		regs->rax=0;
		return;
	}
	u64 address=syscall_sanatize_user_memory(regs->rsi,regs->rdx);
	if (!address){
		regs->rax=0;
		return;
	}
	const drive_t* drive=_get_drive(regs->rdi);
	if (!drive){
		regs->rax=0;
		return;
	}
	partition_flush_cache();
	*((drive_stats_t*)address)=*(drive->stats);
	regs->rax=1;
}
