
#ifndef __CXML_H
#define __CXML_H

#include <stdint.h>
#include <string.h>
#include <errno.h>

/* define as 1 if debug prints are needed in xml_encoder.c */
#define CX_ENC_DBG_EN 0
/* define as 1 if debug prints are needed in xml_decoder.c */
#define CX_DEC_DBG_EN 0

/* if error prints not needed, define as 0 */
#define CX_ERR_PRINT_ALLOWED 1

#if CX_ERR_PRINT_ALLOWED
#define ___cx_pr_err(err) printf (err" @ %s +%u\r\n", __func__, __LINE__)
#else
#define ___cx_pr_err(...)
#endif

#define _cx_lassert(failed, errCode, err) \
	do { \
		if(failed) { \
			___cx_pr_err (err); xStatus = errCode; goto CX_ERR; \
		} \
	} while(0)

#define _cx_func_lassert(function, err) \
	_cx_lassert ((CX_SUCCESS != (xStatus = function)), xStatus, err)

#define _cx_null_lassert(ptr) \
	_cx_lassert ((NULL == ptr), CX_ERR_ALLOC, "Null Buffer: "#ptr)

#define XML_INSTR_STR       "xml version=\"1.0\""

/*define CX_USING_xxx as 1 if the feature xxx should be included*/
#define CX_USING_COMMENTS 0 
#define CX_USING_CDATA    1
#define CX_USING_INSTR    1
#define CX_USING_TAG_ATTR 0

/*can be removed as required.
 *see func addNodeToTree from xml_encoder.c*/
#define STRICT 1

#pragma pack(1)

/*Modifying this requires modifying enum for data types - cxa_type_t;
 * ensure proper format specifiers while encoding different data in xml*/
#define _cx_def_fmts_array(arr) \
	char arr[CXA_MAX][4] = { \
		"%c",    \
		"%s",    \
		"%u",    \
		"%d",    \
		"%hu",   \
		"%hd",   \
		"%u",    \
		"%d",    \
		"%f"     \
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
	({ buf = calloc(1, nBytes); -errno;})

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
static inline char *_cx_strndup(char *src, size_t maxLen, char *dName)
{
	char *dest = NULL;
	size_t dLen, sLen;

	if(!src || !(sLen = strlen (src))) {
		return NULL;
	}

	dLen = ((sLen < maxLen) ? sLen : maxLen);

	_cx_calloc (dest, dLen + 1); /*+1 for NULL char to end-string*/

	if (dest)
		strncpy(dest, src, dLen);
	else
		printf ("%s: Allocating %s: %s\n", __func__, dName, strerror (errno));
}

#endif /*__CXML_H*/
