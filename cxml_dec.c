#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cxml.h"
#include "cxml_api.h"
#include "cxml_errchk.h"

#define SKIP_SPACES(ptr) while (ptr && isspace ((int)*ptr)) { ptr++; }
#define SKIP_LETTERS(ptr) \
	while (ptr && *ptr && \
			!isspace ((int)*ptr) && (*ptr !='/') && (*ptr !='>')) { ptr++; }

#define BET_TOKEN 0x01
#define GET_TOKEN 0x02
static inline char * findStrToken (char *str, const char *token, uint8_t checkType)
{
	char *ptr = strstr (str, token);

	if ((!ptr) || (ptr == str)) {
		return NULL;
	}

	if (checkType & BET_TOKEN) {
		return (char *)!NULL;
	}

	return _cx_strndup (str, (size_t)(ptr - str), str);
}

#define checkStrToken(str, token) findStrToken (str, token, BET_TOKEN)
#define getStrToken(str, token) findStrToken (str, token, GET_TOKEN)

#if CX_USING_TAG_ATTR
static cx_status_t getNodeAttr (cx_node_t *xmlNode, char **tag)
{
	cx_status_t xStatus = CX_SUCCESS;
	cxn_attr_t *curAttr = NULL, *lastAttr = NULL;
	char *ptr, *tPtr, *tEnd;

	cx_null_rfail (*tag);

	tEnd = strchr (*tag, '>');

   	tEnd -= (*(tEnd-1) == '/');
	
	tPtr = *tag;
	ptr = getStrToken (tPtr, "=");
	if (!ptr) {
		cx_dec_dbg ("No attributes for %s", xmlNode->tagField);
		return CX_SUCCESS;
	}

	cx_dec_dbg ("finding attrs for %s", xmlNode->tagField);
	do {
		_cx_calloc (curAttr, sizeof (cxn_attr_t));
		cx_alloc_rfail (curAttr);

		curAttr->attrName = ptr;
		cx_dec_dbg ("attrName: %s", curAttr->attrName);

		ptr = strchr (tPtr, '"');
		cx_lfail ((!ptr) || (!strchr (ptr+1, '"')), CX_ERR_INVALID_XML);

		curAttr->attrValue = getStrToken (++ptr, "\"");
		cx_lfail (!curAttr->attrValue, CX_ERR_NULL_ATTRVALUE);

		cx_dec_dbg ("attrValue: %s", curAttr->attrValue);

		if (!xmlNode->attrList) {
			xmlNode->attrList = curAttr;
		} else {
			lastAttr->next = curAttr;
		}
		lastAttr = curAttr;

		xmlNode->numOfAttr++;
		ptr = strchr (ptr, '"') + 1;
		SKIP_SPACES(ptr);
		tPtr = ptr;
	} while ((tPtr < tEnd) && (NULL != (ptr = getStrToken (tPtr, "="))));

	cx_dec_dbg (" End of attr list for: %s", xmlNode->tagField);
	*tag = tEnd;

CX_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		_cx_free (curAttr->attrName);
		_cx_free (curAttr->attrValue);
		_cx_free (curAttr);
	}
	return xStatus;
}
#endif

static void populateNodeInTree (cx_node_t *prevNode, cx_node_t *curNode)
{
	if (prevNode) {
		if (prevNode->nodeType == CXN_PARENT) {
			if (!prevNode->children) {
				prevNode->children = curNode;
			} else {
				prevNode->lastChild->next = curNode;
			}
			prevNode->lastChild = curNode;
			curNode->parent = prevNode;
			cx_dec_dbg ("'%s' child to '%s'", \
					curNode->tagField, curNode->parent->tagField);
		} else {
			prevNode->next = curNode;
			prevNode->parent->lastChild = curNode;
			curNode->parent = prevNode->parent;
			cx_dec_dbg ("'%s' next to '%s'", \
					prevNode->next->tagField, prevNode->tagField);
		}
	}
}

static cx_status_t getNodeFromNewTag (cx_node_t *prevNode, cx_node_t **curNode, cxn_type_t nodeType, char **_decPtr)
{
	char *decPtr = *_decPtr;
	char *tPtr = decPtr;
	char *tagField = NULL;
	cx_status_t xStatus = CX_SUCCESS;

	/*we dont need check CXN_SINGLE, since it gets updated when '/>' comes*/
	/*after use set decPtr to '>' for other than PARENT;
	 *in PARENT point next to tagName
	 *for content, point to next tag opening*/
	switch (nodeType) {
		case CXN_PARENT:
			cx_dec_dbg ("PARENT: %s", decPtr);
			SKIP_LETTERS(decPtr); /*skip upto end of tag name*/
			cx_rfail (!decPtr, CX_ERR_INVALID_TAG);
			/*copy name to tagField*/
			tagField = _cx_strndup (tPtr, \
					(size_t)(decPtr - tPtr), "Parent");
			break;
#if CX_USING_COMMENTS
		case CXN_COMMENT:
			cx_dec_dbg ("COMMENT: %s", decPtr);
			tagField = getStrToken (decPtr, "-->");
			decPtr = strstr (decPtr, "-->") + 2;
			break;
#endif
#if CX_USING_CDATA
		case CXN_CDATA:
			cx_dec_dbg ("CDATA: %s", decPtr);
			tagField = getStrToken (decPtr, "]]>");
			decPtr = strstr (decPtr, "]]>") + 2;
			break;
#endif
#if CX_USING_INSTR
		case CXN_INSTR:
			cx_dec_dbg ("INSTR: %s", decPtr);
			tagField = getStrToken (decPtr, "?>");
			/*We want decPtr at '>' for now -FIXME*/
			decPtr = strstr (decPtr, "?>") + 1;
#if 1
			/*Just discard any allocations in case it's INSTR tag for now!*/
			*_decPtr = decPtr;
			_cx_free (tagField);
			return CX_SUCCESS;
#else
			/*Fill-up xml version and parsing details -TODO*/
#endif
			break;
#endif
		case CXN_CONTENT:
			cx_dec_dbg ("CONTENT: %s", decPtr);
			tagField = getStrToken (decPtr, "<");
			decPtr = strchr (decPtr, '<');
			break;
		case CXN_SINGLE: /*we won't have this ever, but have fail-safe*/
			break;
		case CXN_MAX:
		default:
			cx_dec_dbg ("Invalid tag type!");
			return CX_ERR_INVALID_TAG;
	}

	cx_null_rfail (tagField);
	cx_rfail (!decPtr, CX_ERR_INVALID_TAG);

	_cx_calloc ((*curNode), sizeof (cx_node_t));
	cx_alloc_rfail (*curNode);

	(*curNode)->tagField = tagField;
	(*curNode)->nodeType = nodeType;
	*_decPtr = decPtr;

	populateNodeInTree (prevNode, *curNode);

	return xStatus;
}

static cx_status_t cx_BuildTreeFromXmlString (cx_cookie_t *cookie)
{
	cx_node_t *curNode = NULL;
	cx_node_t *prevNode = NULL;
	char *decPtr = cookie->xs;
	cx_status_t xStatus = CX_SUCCESS;
	size_t tLen;

NEW_TAG:
	SKIP_SPACES(decPtr);
	if (*decPtr == '<') {
		cxn_type_t type;
		decPtr++;
		if (*decPtr == '/') {
			char *ptr, *tName;

			/*should be a tag-closing: </tagName>*/
			decPtr++;

			ptr = strchr (decPtr, '>');
			cx_rfail (!ptr, CX_ERR_UNCLOSED_TAG);

			cx_rfail (!prevNode, CX_ERR_INVALID_TAG);

			if (curNode->nodeType == CXN_PARENT) {
				tName = curNode->tagField;
			} else if (curNode->parent) {
				tName = curNode->parent->tagField;
				curNode = curNode->parent;
			} else {
				return CX_ERR_LONE_TAG;
			}

			tLen = (size_t)(ptr - decPtr);
			cx_rfail ((strlen (tName) != tLen) || \
					(CX_SUCCESS != strncmp (tName, decPtr, tLen)), \
					CX_ERR_CLOSED_TAG_MISMATCH);

			decPtr = ptr+1;
			cx_dec_dbg ("NODE FULL: %s", tName);
			if (curNode->parent) {
				curNode = curNode->parent;
			}
			prevNode = curNode;
			goto NEW_TAG;
		}
#if CX_USING_INSTR
		else if (*decPtr == '?') {
			/*Instruction tag*/
			decPtr++;
			type = CXN_INSTR;
		}
#endif
#if CX_USING_COMMENTS
		else if (CX_SUCCESS == strncmp (decPtr, "!--", 3)) {
			/*Comment tag*/
			decPtr += 3;
			SKIP_SPACES(decPtr);
			type = CXN_COMMENT;
		}
#endif
#if CX_USING_CDATA
		else if (CX_SUCCESS == strncmp (decPtr, "![CDATA[", 8)) {
			/*Cdata tag*/
			decPtr += 8;
			type = CXN_CDATA;
		}
#endif
		else {
			/*Parent tag.. default*/
			type = CXN_PARENT;
		}

		/*a new tag begins*/
		cx_func_rfail (getNodeFromNewTag (prevNode, &curNode, type, &decPtr));

#if CX_USING_INSTR
		if (!curNode) {
			/*It's INSTR Node, just don't do anything for this*/
			cx_dec_dbg ("Skipping INSTR type node for now..");
			goto TAG_ENDER;
		}
#endif

		cookie->root = (cookie->root) ? cookie->root : curNode;
		prevNode = curNode;

		/*Now decPtr points to <space> or '/' or '>'*/
		cx_dec_dbg ("node: %s", curNode->tagField);
	} else {
		if ((size_t)(decPtr - cookie->xs) == strlen (cookie->xs)) {
			cx_dec_dbg ("DONE!!");
			return CX_SUCCESS;
		}
		/*we have a content of parent now.. not a child*/
		cx_func_rfail (getNodeFromNewTag (prevNode, \
					&curNode, CXN_CONTENT, &decPtr));
		cx_dec_dbg ("content: %s", curNode->tagField);
		/*let prevNode be as it is, this node is just content..
		 *prevNode reference shall be the parent of this content*/
		goto NEW_TAG;
	}

#if CX_USING_INSTR
TAG_ENDER:
#endif
	SKIP_SPACES(decPtr);
	if (*decPtr == '>') { /*Seems tag name ended*/
		/*this parent might have children/content/both/end-of-itself*/
		decPtr++;
		goto NEW_TAG;
	} else if (*decPtr == '/') { /*tag ended is self-tag?*/
		cx_dec_dbg ("self tag: %s", curNode->tagField);
		goto SELF_TAG;
	} else if ((curNode->nodeType == CXN_PARENT)) {
#if CX_USING_TAG_ATTR
		/*if we have attributes, get them*/
		cx_func_rfail (getNodeAttr (curNode, &decPtr));
#endif
	}

	if (*decPtr == '/') {
SELF_TAG:
		/*might be a self-ending-tag*/
		cx_rfail ((*(decPtr+1) !='>'), CX_ERR_INVALID_TAG);
		decPtr++;
		curNode->nodeType = CXN_SINGLE;
		cx_dec_dbg ("single node: %s", curNode->tagField);
	}
	
	if (*decPtr == '>') {
		decPtr++;
	}

	goto NEW_TAG;
}

cx_status_t cx_DecPkt (void **_cookie, char *str, char *name)
{
	cx_status_t xStatus = CX_SUCCESS;
	cx_cookie_t *cookie;

	cx_null_rfail (str);
	cx_rfail ((strlen (str) > CX_MAX_DEC_STR_SZ), CX_ERR_DEC_OVERFLOW);
	cx_rfail (((str = strchr (str, '<')) == NULL), CX_ERR_INVALID_XML);

	_cx_calloc (cookie, sizeof (*cookie));
	cx_alloc_rfail (cookie);

	cookie->cxCode = CX_COOKIE_MAGIC;
	cookie->xs = str;
	cookie->xsIsFromUser = 1;
	strncpy (cookie->name, name, CX_COOKIE_NAMELEN);

	cx_lfail ((xStatus = cx_BuildTreeFromXmlString (cookie)), xStatus);

	*_cookie = cookie;

CX_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		cx_DestroySession (cookie);
	}
	return xStatus;
}
