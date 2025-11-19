/* sys/mnttab.h */
#ifndef _SYS_MNTTAB_H
#define _SYS_MNTTAB_H

/* 
 * Mock structure for Solaris mnttab.
 * Required for libzfs function signatures.
 */
struct mnttab {
    char *mnt_special;
    char *mnt_mountp;
    char *mnt_fstype;
    char *mnt_mntopts;
};

#endif

