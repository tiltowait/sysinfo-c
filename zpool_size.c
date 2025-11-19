#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

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

/*
 * Include ZFS libraries.
 * ensure -I. is used in compilation so it finds your local libshare.h
 */
#include <libnvpair.h>
#include <libzfs.h>
#include "zpool_size.h"

/*
 * =============================================================
 * PUBLIC API
 * =============================================================
 */

/* Get ZFS pool size in bytes using libzfs */
int get_zpool_size(const char *poolname, uint64_t *size) {
  libzfs_handle_t *g_zfs;
  zpool_handle_t *zhp;

  if ((g_zfs = libzfs_init()) == NULL) {
    return -1;
  }

  zhp = zpool_open(g_zfs, poolname);
  if (zhp == NULL) {
    libzfs_fini(g_zfs);
    return -1;
  }

  *size = zpool_get_prop_int(zhp, ZPOOL_PROP_SIZE, NULL);

  zpool_close(zhp);
  libzfs_fini(g_zfs);
  return 0;
}

/*
 * =============================================================
 * STANDALONE UTILITY (when compiled as main program)
 * =============================================================
 */

#ifdef ZPOOL_SIZE_STANDALONE

static int pool_callback(zpool_handle_t *zhp, void *data) {
  uint64_t size;
  char size_str[32];
  const char *pool_name = zpool_get_name(zhp);

  // Get raw size (bytes)
  size = zpool_get_prop_int(zhp, ZPOOL_PROP_SIZE, NULL);

  // Get human-readable size string
  zpool_get_prop(zhp, ZPOOL_PROP_SIZE, size_str, sizeof(size_str), NULL,
                 B_FALSE);

  printf("Pool: %-10s | Size: %s (%lu bytes)\n", pool_name, size_str, size);

  zpool_close(zhp);
  return 0;
}

int main(int argc, char **argv) {
  libzfs_handle_t *g_zfs;

  if ((g_zfs = libzfs_init()) == NULL) {
    fprintf(stderr, "Error: Failed to initialize libzfs.\n");
    return 1;
  }

  zpool_iter(g_zfs, pool_callback, NULL);

  libzfs_fini(g_zfs);
  return 0;
}

#endif /* ZPOOL_SIZE_STANDALONE */
