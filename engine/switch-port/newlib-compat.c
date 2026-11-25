/* SPDX-License-Identifier: MIT
 * Missing POSIX entry points needed by Rust std. No Linux syscall/layout assumptions.
 * libnx thread.c uses 0x1000-byte pages for stack allocation/mapping.
 * Unsupported queries fail, rather than inventing system capabilities.
 */
#include <switch.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
long sysconf(int name) {
    if(name==_SC_PAGESIZE) return 0x1000;
    errno=EINVAL; return -1;
}

/* Back Rust std's secure entropy with Horizon's actual csrng service.
 * The application must successfully initialize csrng before entering Rust.
 */
ssize_t getrandom(void *buffer, size_t length, unsigned int flags) {
    if(flags!=0) { errno=EINVAL; return -1; }
    if(length>SSIZE_MAX) { errno=EINVAL; return -1; }
    if(!length) return 0;
    Result rc=csrngGetRandomBytes(buffer,length);
    if(R_FAILED(rc)) { fprintf(stderr,"csrngGetRandomBytes=%08x\n",rc); errno=EIO; return -1; }
    return (ssize_t)length;
}
