
#ifndef __CXML_H
#define __CXML_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "cxml_cfg.h"

#pragma pack(1)

/* Modifying cx_def_fmts and cx_def_sizes requires
 * modifying enum for data types - cxattr_type_t;
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

typedef struct cxn_attr_s {
    char                *attrName;
    char                *attrValue;
    struct cxn_attr_s   *next;
} cxn_attr_t;

typedef struct cx_node_s {
    uint8_t             nodeType;
    char                *tagField;
    struct cx_node_s    *parent;
#if CX_USING_TAG_ATTR
    uint8_t             numOfAttr;
    cxn_attr_t          *attrList;/*Save tail-ptr to speedup additions -TODO*/
#endif
    struct cx_node_s    *children;
    struct cx_node_s    *lastChild;
    struct cx_node_s    *next;
} __attribute__((__packed__)) cx_node_t;

/**
 * Structure used inside cxml library to identify particular user-cookie to
 * handle corresposing session encode/decode sequence
 * cxCode - Fixed magic number to validate the cookie user has given
 * name - name of the cookie  e.g. light-ctrl, high temp events, etc
 * root - root node of the tree built
 * recent - stores the most recently processed node
 * xmlLength - xmlLength & xstr store all tag/attr strings -TODO
 * uxsLength - user buffer xmlLength & xstr store all tag/attr strings
 * xsIsFromUserR - Indicates if xs is pointing to user-buffer
 * xc - stores all node/attr contents -TODO
 * xs - stores actual xml string
 */
typedef struct cx_cookie_s {
#define CX_COOKIE_MAGIC   0x00C0FFEE
	uint32_t            cxCode;
#define CX_COOKIE_NAMELEN 32
	char                name[CX_COOKIE_NAMELEN];
	cx_node_t           *root;
	cx_node_t           *recent;
	uint32_t            xmlLength;
	uint32_t            uxsLength;
	int                 xsIsFromUser;
	char                *xc;
	char                *xs;
} cx_cookie_t;

/**
 * @func   : _cx_calloc
 * @brief  : allocate memory and fill with 0's if success
 * @called : when memory is required dynamically from heap for large buffers
 * @input  : nBytes - number of bytes for buffer allocation
 * @output : ptr - pointer to hold allocated memory
 * @return : 0 - Success
 *           < 0 - for different failure conditions
 */
#define _cx_calloc(ptr, nBytes) \
	({ ptr = calloc (1, nBytes); -errno;})

/**
 * @func   : _cx_malloc
 * @brief  : allocate memory
 * @called : when memory is required dynamically from heap for large buffers
 * @input  : nBytes - number of bytes for buffer allocation
 * @output : ptr - pointer to hold allocated memory
 * @return : 0 - Success
 *           < 0 - for different failure conditions
 */
#define _cx_malloc(ptr, nBytes) \
	({ ptr = malloc (nBytes); -errno;})

/**
 * @func   : _cx_free
 * @brief  : free an already allocated memory
 * @called : when a dynamically allocated memory is no longer required
 * @input  : ptr - pointer to hold allocated memory
 * @output : none
 * @return : void
 */
#define _cx_free(ptr) \
	do { if (ptr) { free (ptr); ptr = NULL; } } while (0)

char *_cx_strndup (char *src, size_t maxLen, char *dName);

#endif /*__CXML_H*/
