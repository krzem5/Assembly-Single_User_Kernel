#ifndef _KERNEL_VFS_VFS_H_
#define _KERNEL_VFS_VFS_H_ 1
#include <kernel/fs/fs.h>
#include <kernel/id/group.h>
#include <kernel/id/user.h>
#include <kernel/vfs/node.h>



#define VFS_LOOKUP_FLAG_FOLLOW_LINKS 1
#define VFS_LOOKUP_FLAG_CHECK_PERMISSIONS 2



void vfs_mount(filesystem_t* fs,const char* path);



vfs_node_t* vfs_lookup(vfs_node_t* root,const char* path,u32 flags,uid_t uid,gid_t gid);



u32 vfs_path(vfs_node_t* node,char* buffer,u32 buffer_length);



#endif
