#ifndef ARCHIVE_READ_SUPPORT_FORMAT_TAR_PRIVATE_H_INCLUDED
#define	ARCHIVE_READ_SUPPORT_FORMAT_TAR_PRIVATE_H_INCLUDED


/*
 * Layout of POSIX 'ustar' tar header.
 */
struct archive_entry_header_ustar {
	char	name[100];
	char	mode[8];
	char	uid[8];
	char	gid[8];
	char	size[12];
	char	mtime[12];
	char	checksum[8];
	char	typeflag[1];
	char	linkname[100];	/* "old format" header ends here */
	char	magic[6];	/* For POSIX: "ustar\0" */
	char	version[2];	/* For POSIX: "00" */
	char	uname[32];
	char	gname[32];
	char	rdevmajor[8];
	char	rdevminor[8];
	char	prefix[155];
};

/*
 * Structure of GNU tar header
 */
struct gnu_sparse {
	char	offset[12];
	char	numbytes[12];
};

struct archive_entry_header_gnutar {
	char	name[100];
	char	mode[8];
	char	uid[8];
	char	gid[8];
	char	size[12];
	char	mtime[12];
	char	checksum[8];
	char	typeflag[1];
	char	linkname[100];
	char	magic[8];  /* "ustar  \0" (note blank/blank/null at end) */
	char	uname[32];
	char	gname[32];
	char	rdevmajor[8];
	char	rdevminor[8];
	char	atime[12];
	char	ctime[12];
	char	offset[12];
	char	longnames[4];
	char	unused[1];
	struct gnu_sparse sparse[4];
	char	isextended[1];
	char	realsize[12];
	/*
	 * Old GNU format doesn't use POSIX 'prefix' field; they use
	 * the 'L' (longname) entry instead.
	 */
};

unsigned char * to_unsigned_char_ptr(void * p)
{
    return (unsigned char *) p;
}

struct archive_entry_header_ustar * to_archive_entry_header_ustar_ptr(void * p)
{
    return (struct archive_entry_header_ustar *) p;
}

struct archive_entry_header_gnutar * to_archive_entry_header_gnutar_ptr(void * p)
{
    return (struct archive_entry_header_gnutar *) p;
}

#endif
