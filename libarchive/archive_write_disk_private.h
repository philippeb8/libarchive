/*-
 * Copyright (c) 2003-2007 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: head/lib/libarchive/archive_write_disk_private.h 201086 2009-12-28 02:17:53Z kientzle $
 */

#ifndef __LIBARCHIVE_BUILD
#error This header is only to be used internally to libarchive.
#endif

#ifndef ARCHIVE_WRITE_DISK_PRIVATE_H_INCLUDED
#define ARCHIVE_WRITE_DISK_PRIVATE_H_INCLUDED

#include "archive_acl_private.h"
#include "archive_private.h"
#include "archive_entry_private.h"

struct fixup_entry {
	struct fixup_entry	*next;
	struct archive_acl	 acl;
	mode_t			 mode;
	int64_t			 atime;
	int64_t                  birthtime;
	int64_t			 mtime;
	int64_t			 ctime;
	unsigned long		 atime_nanos;
	unsigned long            birthtime_nanos;
	unsigned long		 mtime_nanos;
	unsigned long		 ctime_nanos;
	unsigned long		 fflags_set;
	size_t			 mac_metadata_size;
	void			*mac_metadata;
	int			 fixup; /* bitmask of what needs fixing */
	char			*name;
};

struct archive_write_disk {
	struct archive	archive;

	mode_t			 user_umask;
	struct fixup_entry	*fixup_list;
	struct fixup_entry	*current_fixup;
	int64_t			 user_uid;
	int			 skip_file_set;
	int64_t			 skip_file_dev;
	int64_t			 skip_file_ino;
	time_t			 start_time;

	int64_t (*lookup_gid)(void *private, const char *gname, int64_t gid);
	void  (*cleanup_gid)(void *private);
	void			*lookup_gid_data;
	int64_t (*lookup_uid)(void *private, const char *uname, int64_t uid);
	void  (*cleanup_uid)(void *private);
	void			*lookup_uid_data;

	/*
	 * Full path of last file to satisfy symlink checks.
	 */
	struct archive_string	path_safe;

	/*
	 * Cached stat data from disk for the current entry.
	 * If this is valid, pst points to st.  Otherwise,
	 * pst is null.
	 */
	struct stat		 st;
	struct stat		*pst;

	/* Information about the object being restored right now. */
	struct archive_entry	*entry; /* Entry being extracted. */
	char			*name; /* Name of entry, possibly edited. */
	struct archive_string	 _name_data; /* backing store for 'name' */
	/* Tasks remaining for this object. */
	int			 todo;
	/* Tasks deferred until end-of-archive. */
	int			 deferred;
	/* Options requested by the client. */
	int			 flags;
	/* Handle for the file we're restoring. */
	int			 fd;
	/* Current offset for writing data to the file. */
	int64_t			 offset;
	/* Last offset actually written to disk. */
	int64_t			 fd_offset;
	/* Total bytes actually written to files. */
	int64_t			 total_bytes_written;
	/* Maximum size of file, -1 if unknown. */
	int64_t			 filesize;
	/* Dir we were in before this restore; only for deep paths. */
	int			 restore_pwd;
	/* Mode we should use for this entry; affected by _PERM and umask. */
	mode_t			 mode;
	/* UID/GID to use in restoring this entry. */
	int64_t			 uid;
	int64_t			 gid;
	/*
	 * HFS+ Compression.
	 */
	/* Xattr "com.apple.decmpfs". */
	uint32_t		 decmpfs_attr_size;
	unsigned char		*decmpfs_header_p;
	/* ResourceFork set options used for fsetxattr. */
	int			 rsrc_xattr_options;
	/* Xattr "com.apple.ResourceFork". */
	unsigned char		*resource_fork;
	size_t			 resource_fork_allocated_size;
	unsigned int		 decmpfs_block_count;
	uint32_t		*decmpfs_block_info;
	/* Buffer for compressed data. */
	unsigned char		*compressed_buffer;
	size_t			 compressed_buffer_size;
	size_t			 compressed_buffer_remaining;
	/* The offset of the ResourceFork where compressed data will
	 * be placed. */
	uint32_t		 compressed_rsrc_position;
	uint32_t		 compressed_rsrc_position_v;
	/* Buffer for uncompressed data. */
	char			*uncompressed_buffer;
	size_t			 block_remaining_bytes;
	size_t			 file_remaining_bytes;
#ifdef HAVE_ZLIB_H
	z_stream		 stream;
	int			 stream_valid;
	int			 decmpfs_compression_level;
#endif
};


int
archive_write_disk_set_acls(struct archive *, int /* fd */, const char * /* pathname */, struct archive_acl *);

#endif
