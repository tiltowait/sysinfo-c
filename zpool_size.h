#ifndef ZPOOL_SIZE_H
#define ZPOOL_SIZE_H

#include <stdint.h>

/*
 * Get ZFS pool size in bytes using libzfs.
 *
 * Parameters:
 *   poolname - Name of the ZFS pool (e.g., "zroot")
 *   size     - Pointer to store the pool size in bytes
 *
 * Returns:
 *   0 on success, -1 on error
 */
int get_zpool_size(const char *poolname, uint64_t *size);

#endif /* ZPOOL_SIZE_H */
