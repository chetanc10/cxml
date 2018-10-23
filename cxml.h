
#ifndef __CXML_H
#define __CXML_H

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "cxml_cfg.h"

#pragma pack(1)

/*Modifying cx_def_fmts and cx_def_sizes requires modifying enum for data types - cxattr_type_t;
 * ensure proper format specifiers while encoding different data in xml*/
#define _cx_def_fmts_array(arr) \
	char arr[CXATTR_MAX][4] = { \
		"%s",                   \
		"%c",                   \
		"%u",                   \
		"%d",                   \
		"%hu",                  \
		"%hd",                  \
		"%u",                   \
		"%d",                   \
		"%f"                    \
	};
#define _cx_def_size_array(arr) \
	size_t arr[CXATTR_MAX] = {  \
		0,                      \
		sizeof (char),          \
		sizeof (uint8_t),       \
		sizeof (int8_t),        \
		sizeof (uint16_t),      \
		sizeof (int16_t),       \
		sizeof (uint32_t),      \
		sizeof (int32_t),       \
		sizeof (float),         \
	};

#define NAME2STR(x) (#x)

/**
 * @func   : _cx_calloc
 * @brief  : allocate memory and fill with 0's if success
 * @called : when memory is required dynamically from heap for large buffers
 * @input  : nBytes - number of bytes for buffer allocation
 * @output : buf - pointer to hold allocated memory
 * @return : 0 - Success
 *           < 0 - for different failure conditions
 */
#define _cx_calloc(buf, nBytes) \
	({ buf = calloc (1, nBytes); -errno;})

#define _cx_free(buf) \
	do { if (buf) free (buf); } while (0)

/**
 * @func   : _cx_strndup
 * @brief  : safely duplicate a source string using length limits specified
 * @called : called to duplicate an existing string into a memory from heap
 * @input  : char *src - src string ptr
 *           size_t maxLen - maximum allowed length to use during duplication
 *           char *dName - name to relate to duplicated buffer
 * @output : none
 * @return : NULL - Failure or for NULL src or 0 sized src
 *           < 0 - for different failure conditions
 */
static inline char *_cx_strndup (char *src, size_t maxLen, char *dName)
{
	char *dest = NULL;
	size_t dLen, sLen;

	if (!src || !(sLen = strlen (src))) {
		return NULL;
	}

	dLen = ((sLen < maxLen) ? sLen : maxLen);

	_cx_calloc (dest, dLen + 1); /*+1 for NULL char to end-string*/

	if (dest)
		strncpy (dest, src, dLen);
	else
		printf ("%s: Allocating %s: %s\n", __func__, dName, strerror (errno));
}

#endif /*__CXML_H*/
