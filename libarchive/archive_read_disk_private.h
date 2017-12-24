/*-
 * Copyright (c) 2003-2009 Tim Kientzle
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
 * $FreeBSD: head/lib/libarchive/archive_read_disk_private.h 201105 2009-12-28 03:20:54Z kientzle $
 */

#ifndef __LIBARCHIVE_BUILD
#error This header is only to be used internally to libarchive.
#endif

struct __dirstream { int i; };

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifndef ARCHIVE_READ_DISK_PRIVATE_H_INCLUDED
#define ARCHIVE_READ_DISK_PRIVATE_H_INCLUDED

struct tree;
struct archive_entry;

#if !defined(_WIN32) || defined(__CYGWIN__)
/*
 * Local data for this package.
 */
struct entry_sparse {
        int64_t		 length;
        int64_t		 offset;
};

struct restore_time {
	const char		*name;
	time_t			 mtime;
	long			 mtime_nsec;
	time_t			 atime;
	long			 atime_nsec;
	mode_t			 filetype;
	int			 noatime;
};

struct tree_entry {
	int			 depth;
	struct tree_entry	*next;
	struct tree_entry	*parent;
	struct archive_string	 name;
	size_t			 dirname_length;
	int64_t			 dev;
	int64_t			 ino;
	int			 flags;
	int			 filesystem_id;
	/* How to return back to the parent of a symlink. */
	int			 symlink_parent_fd;
	/* How to restore time of a directory. */
	struct restore_time	 restore_time;
};

struct filesystem {
	int64_t		dev;
	int		synthetic;
	int		remote;
	int		noatime;
#if defined(USE_READDIR_R)
	size_t		name_max;
#endif
	long		incr_xfer_size;
	long		max_xfer_size;
	long		min_xfer_size;
	long		xfer_align;

	/*
	 * Buffer used for reading file contents.
	 */
	/* Exactly allocated memory pointer. */
	unsigned char	*allocation_ptr;
	/* Pointer adjusted to the filesystem alignment . */
	unsigned char	*buff;
	size_t		 buff_size;
};

struct tree {
	struct tree_entry	*stack;
	struct tree_entry	*current;
	DIR			*d;
#define	INVALID_DIR_HANDLE NULL
	struct dirent		*de;
#if defined(USE_READDIR_R)
	struct dirent		*dirent;
	size_t			 dirent_allocated;
#endif
	int			 flags;
	int			 visit_type;
	/* Error code from last failed operation. */
	int			 tree_errno;

	/* Dynamically-sized buffer for holding path */
	struct archive_string	 path;

	/* Last path element */
	const char		*basename;
	/* Leading dir length */
	size_t			 dirname_length;

	int			 depth;
	int			 openCount;
	int			 maxOpenCount;
	int			 initial_dir_fd;
	int			 working_dir_fd;

	struct stat		 lst;
	struct stat		 st;
	int			 descend;
	int			 nlink;
	/* How to restore time of a file. */
	struct restore_time	 restore_time;

	struct entry_sparse     *sparse_list, *current_sparse;
	int			 sparse_count;
	int			 sparse_list_size;

	char			 initial_symlink_mode;
	char			 symlink_mode;
	struct filesystem	*current_filesystem;
	struct filesystem	*filesystem_table;
	int			 initial_filesystem_id;
	int			 current_filesystem_id;
	int			 max_filesystem_id;
	int			 allocated_filesytem;

	int			 entry_fd;
	int			 entry_eof;
	int64_t			 entry_remaining_bytes;
	int64_t			 entry_total;
	unsigned char		*entry_buff;
	size_t			 entry_buff_size;
};
#endif

struct archive_read_disk {
	struct archive	archive;

	/* Reused by archive_read_next_header() */
	struct archive_entry *entry;

	/*
	 * Symlink mode is one of 'L'ogical, 'P'hysical, or 'H'ybrid,
	 * following an old BSD convention.  'L' follows all symlinks,
	 * 'P' follows none, 'H' follows symlinks only for the first
	 * item.
	 */
	char	symlink_mode;

	/*
	 * Since symlink interaction changes, we need to track whether
	 * we're following symlinks for the current item.  'L' mode above
	 * sets this true, 'P' sets it false, 'H' changes it as we traverse.
	 */
	char	follow_symlinks;  /* Either 'L' or 'P'. */

	/* Directory traversals. */
	struct tree *tree;
	int	(*open_on_current_dir)(struct tree*, const char *, int);
	int	(*tree_current_dir_fd)(struct tree*);
	int	(*tree_enter_working_dir)(struct tree*);

	/* Set 1 if users request to restore atime . */
	int		 restore_time;
	/* Set 1 if users request to honor nodump flag . */
	int		 honor_nodump;
	/* Set 1 if users request to enable mac copyfile. */
	int		 enable_copyfile;
	/* Set 1 if users request to traverse mount points. */
	int		 traverse_mount_points;
	/* Set 1 if users want to suppress xattr information. */
	int		 suppress_xattr;

	const char * (*lookup_gname)(void *private, int64_t gid);
	void	(*cleanup_gname)(void *private);
	void	 *lookup_gname_data;
	const char * (*lookup_uname)(void *private, int64_t uid);
	void	(*cleanup_uname)(void *private);
	void	 *lookup_uname_data;

	int	(*metadata_filter_func)(struct archive *, void *,
			struct archive_entry *);
	void	*metadata_filter_data;

	/* ARCHIVE_MATCH object. */
	struct archive	*matching;
	/* Callback function, this will be invoked when ARCHIVE_MATCH
	 * archive_match_*_excluded_ae return true. */
	void	(*excluded_cb_func)(struct archive *, void *,
			 struct archive_entry *);
	void	*excluded_cb_data;
};

#endif
