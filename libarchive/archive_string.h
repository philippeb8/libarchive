/*-
 * Copyright (c) 2003-2010 Tim Kientzle
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
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
 * $FreeBSD: head/lib/libarchive/archive_string.h 201092 2009-12-28 02:26:06Z kientzle $
 *
 */

#ifndef __LIBARCHIVE_BUILD
#ifndef __LIBARCHIVE_TEST
#error This header is only to be used internally to libarchive.
#endif
#endif

#ifndef ARCHIVE_STRING_H_INCLUDED
#define	ARCHIVE_STRING_H_INCLUDED

#include <stdarg.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>  /* required for wchar_t on some systems */
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif

#include "archive.h"

struct archive_string_conv;

typedef int (*converter_)(struct archive_string *, const void *, size_t,
    struct archive_string_conv *);

struct archive_string_conv {
	struct archive_string_conv	*next;
	char				*from_charset;
	char				*to_charset;
	unsigned			 from_cp;
	unsigned			 to_cp;
	/* Set 1 if from_charset and to_charset are the same. */
	int				 same;
	int				 flag;
#define SCONV_TO_CHARSET	1	/* MBS is being converted to specified
					 * charset. */
#define SCONV_FROM_CHARSET	(1<<1)	/* MBS is being converted from
					 * specified charset. */
#define SCONV_BEST_EFFORT 	(1<<2)	/* Copy at least ASCII code. */
#define SCONV_WIN_CP	 	(1<<3)	/* Use Windows API for converting
					 * MBS. */
#define SCONV_UTF8_LIBARCHIVE_2 (1<<4)	/* Incorrect UTF-8 made by libarchive
					 * 2.x in the wrong assumption. */
#define SCONV_NORMALIZATION_C	(1<<6)	/* Need normalization to be Form C.
					 * Before UTF-8 characters are actually
					 * processed. */
#define SCONV_NORMALIZATION_D	(1<<7)	/* Need normalization to be Form D.
					 * Before UTF-8 characters are actually
					 * processed.
					 * Currently this only for MAC OS X. */
#define SCONV_TO_UTF8		(1<<8)	/* "to charset" side is UTF-8. */
#define SCONV_FROM_UTF8		(1<<9)	/* "from charset" side is UTF-8. */
#define SCONV_TO_UTF16BE 	(1<<10)	/* "to charset" side is UTF-16BE. */
#define SCONV_FROM_UTF16BE 	(1<<11)	/* "from charset" side is UTF-16BE. */
#define SCONV_TO_UTF16LE 	(1<<12)	/* "to charset" side is UTF-16LE. */
#define SCONV_FROM_UTF16LE 	(1<<13)	/* "from charset" side is UTF-16LE. */
#define SCONV_TO_UTF16		(SCONV_TO_UTF16BE | SCONV_TO_UTF16LE)
#define SCONV_FROM_UTF16	(SCONV_FROM_UTF16BE | SCONV_FROM_UTF16LE)

#if HAVE_ICONV
	iconv_t				 cd;
	iconv_t				 cd_w;/* Use at archive_mstring on
				 	       * Windows. */
#endif
	/* A temporary buffer for normalization. */
	struct archive_string		 utftmp;
	converter_                       converter[2];
	int				 nconverter;
};

/* Initialize an archive_string object on the stack or elsewhere. */
#define	archive_string_init(a)	\
	do { (a)->s = NULL; (a)->length = 0; (a)->buffer_length = 0; } while(0)

/* Append a C char to an archive_string, resizing as necessary. */
struct archive_string *
archive_strappend_char(struct archive_string *, char);

/* Ditto for a wchar_t and an archive_wstring. */
struct archive_wstring *
archive_wstrappend_wchar(struct archive_wstring *, wchar_t);

/* Convert a Unicode string to current locale and append the result. */
/* Returns -1 if conversion fails. */
int
archive_string_append_from_wcs(struct archive_string *, const wchar_t *, size_t);


/* Create a string conversion object.
 * Return NULL and set a error message if the conversion is not supported
 * on the platform. */
struct archive_string_conv *
archive_string_conversion_to_charset(struct archive *, const char *, int);
struct archive_string_conv *
archive_string_conversion_from_charset(struct archive *, const char *, int);
/* Create the default string conversion object for reading/writing an archive.
 * Return NULL if the conversion is unneeded.
 * Note: On non Windows platform this always returns NULL.
 */
struct archive_string_conv *
archive_string_default_conversion_for_read(struct archive *);
struct archive_string_conv *
archive_string_default_conversion_for_write(struct archive *);
/* Dispose of a string conversion object. */
void
archive_string_conversion_free(struct archive *);
const char *
archive_string_conversion_charset_name(struct archive_string_conv *);
void
archive_string_conversion_set_opt(struct archive_string_conv *, int);
#define SCONV_SET_OPT_UTF8_LIBARCHIVE2X	1
#define SCONV_SET_OPT_NORMALIZATION_C	2
#define SCONV_SET_OPT_NORMALIZATION_D	4


/* Copy one archive_string to another in locale conversion.
 * Return -1 if conversion failes. */
int
archive_strncpy_l(struct archive_string *, const void *, size_t,
    struct archive_string_conv *);

/* Copy one archive_string to another in locale conversion.
 * Return -1 if conversion failes. */
int
archive_strncat_l(struct archive_string *, const void *, size_t,
    struct archive_string_conv *);


/* Copy one archive_string to another */
#define	archive_string_copy(dest, src) \
	((dest)->length = 0, archive_string_concat((dest), (src)))
#define	archive_wstring_copy(dest, src) \
	((dest)->length = 0, archive_wstring_concat((dest), (src)))

/* Concatenate one archive_string to another */
void archive_string_concat(struct archive_string *dest, struct archive_string *src);
void archive_wstring_concat(struct archive_wstring *dest, struct archive_wstring *src);

/* Ensure that the underlying buffer is at least as large as the request. */
struct archive_string *
archive_string_ensure(struct archive_string *, size_t);
struct archive_wstring *
archive_wstring_ensure(struct archive_wstring *, size_t);

/* Append C string, which may lack trailing \0. */
/* The source is declared void * here because this gets used with
 * "signed char *", "unsigned char *" and "char *" arguments.
 * Declaring it "char *" as with some of the other functions just
 * leads to a lot of extra casts. */
struct archive_string *
archive_strncat(struct archive_string *, const void *, size_t);
struct archive_wstring *
archive_wstrncat(struct archive_wstring *, const wchar_t *, size_t);

/* Append a C string to an archive_string, resizing as necessary. */
struct archive_string *
archive_strcat(struct archive_string *, const void *);
struct archive_wstring *
archive_wstrcat(struct archive_wstring *, const wchar_t *);

/* Copy a C string to an archive_string, resizing as necessary. */
#define	archive_strcpy(as,p) \
	archive_strncpy((as), (p), ((p) == NULL ? 0 : strlen(p)))
#define	archive_wstrcpy(as,p) \
	archive_wstrncpy((as), (p), ((p) == NULL ? 0 : wcslen(p)))
#define	archive_strcpy_l(as,p,lo) \
	archive_strncpy_l((as), (p), ((p) == NULL ? 0 : strlen(p)), (lo))

/* Copy a C string to an archive_string with limit, resizing as necessary. */
#define	archive_strncpy(as,p,l) \
	((as)->length=0, archive_strncat((as), (p), (l)))
#define	archive_wstrncpy(as,p,l) \
	((as)->length = 0, archive_wstrncat((as), (p), (l)))

/* Return length of string. */
#define	archive_strlen(a) ((a)->length)

/* Set string length to zero. */
#define	archive_string_empty(a) ((a)->length = 0)
#define	archive_wstring_empty(a) ((a)->length = 0)

/* Release any allocated storage resources. */
void	archive_string_free(struct archive_string *);
void	archive_wstring_free(struct archive_wstring *);

/* Like 'vsprintf', but resizes the underlying string as necessary. */
/* Note: This only implements a small subset of standard printf functionality. */
void	archive_string_vsprintf(struct archive_string *, const char *,
	    va_list) __LA_PRINTF(2, 0);
void	archive_string_sprintf(struct archive_string *, const char *, ...)
	    __LA_PRINTF(2, 3);

/* Translates from MBS to Unicode. */
/* Returns non-zero if conversion failed in any way. */
int archive_wstring_append_from_mbs(struct archive_wstring *dest,
    const char *, size_t);


void	archive_mstring_clean(struct archive_mstring *);
void	archive_mstring_copy(struct archive_mstring *dest, struct archive_mstring *src);
int archive_mstring_get_mbs(struct archive *, struct archive_mstring *, const char **);
int archive_mstring_get_utf8(struct archive *, struct archive_mstring *, const char **);
int archive_mstring_get_wcs(struct archive *, struct archive_mstring *, const wchar_t **);
int	archive_mstring_get_mbs_l(struct archive_mstring *, const char **,
	    size_t *, struct archive_string_conv *);
int	archive_mstring_copy_mbs(struct archive_mstring *, const char *mbs);
int	archive_mstring_copy_mbs_len(struct archive_mstring *, const char *mbs,
	    size_t);
int	archive_mstring_copy_utf8(struct archive_mstring *, const char *utf8);
int	archive_mstring_copy_wcs(struct archive_mstring *, const wchar_t *wcs);
int	archive_mstring_copy_wcs_len(struct archive_mstring *,
	    const wchar_t *wcs, size_t);
int	archive_mstring_copy_mbs_len_l(struct archive_mstring *,
	    const char *mbs, size_t, struct archive_string_conv *);
int     archive_mstring_update_utf8(struct archive *, struct archive_mstring *aes, const char *utf8);


#endif
