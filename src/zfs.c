#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/*
 * =============================================================
 * SOLARIS COMPATIBILITY LAYER
 * Required because standard FreeBSD headers do not define these
 * legacy Solaris types expected by OpenZFS headers.
 * =============================================================
 */
typedef unsigned int uint_t;
typedef unsigned char uchar_t;
typedef unsigned long ulong_t;
typedef long long hrtime_t;

#ifndef _BOOLEAN_T_DECLARED
typedef enum { B_FALSE, B_TRUE } boolean_t;
#define _BOOLEAN_T_DECLARED
#endif

#include "zfs.h"
#include <libnvpair.h>
#include <libzfs.h>

/* Get ZFS pool size in bytes using libzfs */
int get_zpool_size(uint64_t *size) {
  libzfs_handle_t *g_zfs;
  zpool_handle_t *zhp;

  if ((g_zfs = libzfs_init()) == NULL) {
    return -1;
  }

  zhp = zpool_open(g_zfs, "zroot");
  if (zhp == NULL) {
    libzfs_fini(g_zfs);
    return -1;
  }

  *size = zpool_get_prop_int(zhp, ZPOOL_PROP_SIZE, NULL);

  zpool_close(zhp);
  libzfs_fini(g_zfs);

  return 0;
}
