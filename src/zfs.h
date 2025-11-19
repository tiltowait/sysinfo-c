#ifndef ZFS_H
#define ZFS_H

#include <stdint.h>

/*
 * Get ZFS pool size in bytes using libzfs.
 * Queries the 'zroot' pool.
 *
 * Parameters:
 *   size - Pointer to store the pool size in bytes
 *
 * Returns:
 *   0 on success, -1 on error
 */
int get_zpool_size(uint64_t *size);

#endif /* ZFS_H */
