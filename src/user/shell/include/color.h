#ifndef _COLOR_H_
#define _COLOR_H_ 1
#include <user/fd.h>
#include <user/types.h>



void color_print_file_name(const fd_stat_t* stat,const char* name,s64 parent_fd,s64 fd);



#endif