/* SPDX-License-Identifier: MIT. Preserve complete error lines without flushing each formatting fragment. */
#pragma once
#include <stdio.h>
#include <string.h>
static inline int nx_log_emit(FILE *file,const char *data,size_t size,size_t *written){
 if(!file){*written=0;return -1;}
 *written=fwrite(data,1,size,file);if(*written!=size)return -1;
 return memchr(data,'\n',size)?fflush(file):0;
}
