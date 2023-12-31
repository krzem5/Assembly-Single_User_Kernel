#include <kernel/initramfs/drive.h>
#include <kernel/initramfs/fs.h>
#include <kernel/initramfs/partition.h>
#include <kernel/log/log.h>
#include <kernel/types.h>
#define KERNEL_LOG_NAME "initramfs"



KERNEL_INIT(){
	LOG("Loading initramfs...");
	initramfs_fs_init();
	initramfs_partition_init();
	initramfs_drive_init();
}
