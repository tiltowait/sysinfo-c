/* libshare.h */
#ifndef _LIBSHARE_H
#define _LIBSHARE_H

/*
 * Defines dummy types for libshare structures.
 * This allows libzfs.h to compile without the real internal header.
 */
typedef void * sa_share_t;

/* Dummy enum for sa_protocol to silence compiler warnings */
enum sa_protocol {
	SA_PROTOCOL_NFS = 0,
	SA_PROTOCOL_SMB = 1
};

#endif

