#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cxml.h"
#include "cxml_api.h"
#include "cxml_errchk.h"

#if CX_DEC_DBG_EN
#define cx_dec_dbg printf
#else
#define cx_dec_dbg(...)
#endif

#define SKIP_SPACES(ptr) while (ptr && isspace ((int)*ptr)) { ptr++; }
#define SKIP_LETTERS(ptr) \
	while (ptr && *ptr && !isspace ((int)*ptr) && (*ptr !='/') && (*ptr !='>')) { ptr++; }

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
	cxn_attr_t *curAttr = NULL, *lastAttr = NULL;
	char *ptr, *tPtr, *tEnd;

	if (!(*tag)) {
		printf ("What String is this?\n");
		return CX_ERR_BAD_TAG;
	} else {
		tEnd = strchr (*tag, '>');
	}

   	tEnd -= (*(tEnd-1) == '/');
	
	tPtr = *tag;
	ptr = getStrToken (tPtr, "=");
	if (!ptr) {
		printf ("No attributes for %s\n", xmlNode->tagField);
		return CX_SUCCESS;
	}

	cx_dec_dbg ("finding attrs for %s\n", xmlNode->tagField);
	do {
		_cx_calloc (curAttr, sizeof (cxn_attr_t));
		if (!curAttr) {
			printf ("doomed memory\n");
			return CX_ERR_ALLOC;
		}

		curAttr->attrName = ptr;
		cx_dec_dbg ("%s=", curAttr->attrName);

		ptr = strchr (tPtr, '=');
		if (!ptr) {
			cx_dec_dbg ("No more attributes\n");
			break;
		}
		ptr = strchr (ptr, '"');
		if ((!ptr) || (!strchr (ptr+1, '"'))) {
			printf ("Give something proper to chew!!\n");
			return CX_ERR_BAD_ATTR;
		}

		curAttr->attrValue = getStrToken (++ptr, "\"");
		if (!curAttr->attrValue) {
			printf ("Error getting attrValue\n");
			return CX_ERR_BAD_ATTR;
		}
		cx_dec_dbg ("%s ", curAttr->attrValue);

		if (!xmlNode->attrList) {
			xmlNode->attrList = curAttr;
		} else {
			lastAttr->next = curAttr;
		}
		lastAttr = curAttr;

		xmlNode->numOfAttr++;
		SKIP_LETTERS(ptr);
		SKIP_SPACES(ptr);
		tPtr = ptr;
	} while ((tPtr < tEnd) && (NULL != (ptr = getStrToken (tPtr, "="))));

	cx_dec_dbg (" | attr listed for: %s\n", xmlNode->tagField);
	*tag = tEnd;

	return CX_SUCCESS;
}
#endif

static void populateNodeInTree (cx_node_t *prevNode, cx_node_t *curNode)
{
	if (prevNode) {
		if (prevNode->nodeType == CXN_PARENT) {
			cx_dec_dbg ("add as child\n");
			if (!prevNode->children) {
				cx_dec_dbg ("1st child\n");
				prevNode->children = curNode;
			} else {
				prevNode->lastChild->next = curNode;
			}
			prevNode->lastChild = curNode;
			curNode->parent = prevNode;
			cx_dec_dbg ("%s child to %s\n", curNode->tagField, curNode->parent->tagField);
		} else {
			prevNode->next = curNode;
			prevNode->parent->lastChild = curNode;
			curNode->parent = prevNode->parent;
			cx_dec_dbg ("%s next to %s\n", prevNode->next->tagField, prevNode->tagField);
		}
	}
}

static cx_status_t getNodeFromNewTag (cx_node_t *prevNode, cx_node_t **curNode, cxn_type_t nodeType, char **_decPtr)
{
	char *decPtr = *_decPtr;
	char *tPtr = decPtr;
	char *tagField;

	_cx_calloc ((*curNode), sizeof (cx_node_t));
	if (!(*curNode)) {
		printf ("memory alloc err for node\n");
		return CX_ERR_ALLOC;
	}

	/*we dont need check CXN_SINGLE, since it gets updated when '/>' comes*/
	/*after use set decPtr to '>' for other than PARENT;
	 *in PARENT point next to tagName
	 *for content, point to next tag opening*/
	switch (nodeType) {
		case CXN_PARENT:
			cx_dec_dbg ("PARENT: ");
			cx_dec_dbg ("%s", decPtr);
			SKIP_LETTERS(decPtr); /*skip upto end of tag name*/
			if (!*decPtr) {
				printf ("Bad Tag.. not XML format!\r\n");
				goto OFF_TAG;
			}
			/*copy name to tagField*/
			tagField = _cx_strndup (tPtr, \
					(size_t)(decPtr - tPtr), "Parent");
			break;
#if CX_USING_COMMENTS
		case CXN_COMMENT:
			cx_dec_dbg ("COMMENT\n");
			tagField = getStrToken (decPtr, "-->");
			decPtr = strstr (decPtr, "-->") + 2;
			break;
#endif
#if CX_USING_CDATA
		case CXN_CDATA:
			cx_dec_dbg ("CDATA\n");
			tagField = getStrToken (decPtr, "]]>");
			decPtr = strstr (decPtr, "]]>") + 2;
			break;
#endif
#if CX_USING_INSTR
		case CXN_INSTR:
			cx_dec_dbg ("INSTR\n");
			tagField = getStrToken (decPtr, "?>");
			decPtr = strstr (decPtr, "?>") + 1; /*We want decPtr at '>' for now -FIXME*/
#if 1
			/*Just discard any allocations in case it's INSTR tag for now!*/
			*_decPtr = decPtr;
			_cx_free (tagField);
			_cx_free (*curNode);
			return CX_SUCCESS;
#else
			/*Fill-up xml version and parsing details -TODO*/
#endif
			break;
#endif
		case CXN_CONTENT:
			cx_dec_dbg ("CONTENT\n");
			tagField = getStrToken (decPtr, "<");
			decPtr = strchr (decPtr, '<');
			break;
		case CXN_SINGLE: /*we won't have this ever, but have fail-safe*/
			break;
		case CXN_MIN:
		case CXN_MAX:
		default:
			_cx_free ((*curNode));
			return CX_ERR_BAD_NODETYPE;
	}

	if (!tagField) {
		printf ("tagField memory error\n");
OFF_TAG:
		_cx_free ((*curNode));
		return CX_ERR_ALLOC;
	}

	(*curNode)->tagField = tagField;
	(*curNode)->nodeType = nodeType;
	*_decPtr = decPtr;

	populateNodeInTree (prevNode, *curNode);

	return CX_SUCCESS;
}

static cx_status_t cx_BuildTreeFromXmlString (cx_cookie_t *cookie)
{
	cx_node_t *curNode = NULL;
	cx_node_t *prevNode = NULL;
	char *decPtr = cookie->xs;
	cx_status_t status;

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
			if (!ptr) {
				printf ("Syntax error: bad tag-closing\n");
				return CX_ERR_BAD_TAG;
			} else if (!prevNode) {
				printf ("Syntax error: bad tag-closing.. no prev\n");
				return CX_ERR_BAD_TAG;
			}

			if (curNode->nodeType == CXN_PARENT) {
				tName = curNode->tagField;
			} else if (curNode->parent) {
				tName = curNode->parent->tagField;
				curNode = curNode->parent;
			} else {
				printf ("Unknown error\n");
				return CX_FAILURE;
			}

			if ((strlen (tName) != (size_t)(ptr - decPtr)) || \
					(CX_SUCCESS != strncmp (tName, decPtr, (size_t)(ptr-decPtr)))) {
				printf ("NODE Mismatch at closing-tag: %s\n", tName);
				return CX_ERR_BAD_TAG;
			}

			decPtr = ptr+1;
			cx_dec_dbg ("NODE FULL: %s\n", tName);
			if (curNode->parent) {
				prevNode = curNode = curNode->parent;
			} else {
				prevNode = curNode;
			}
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
		status = getNodeFromNewTag (prevNode, &curNode, type, &decPtr);
		if (CX_SUCCESS != status) {
			printf ("node addition failed\n");
			return status;
		}
#if CX_USING_INSTR
		if (!curNode) {
			/*It's INSTR Node, just don't do anything for this*/
			cx_dec_dbg ("Skipping INSTR type node for now..\n");
			goto TAG_ENDER;
		}
#endif

		cookie->root = (cookie->root) ? cookie->root : curNode;
		prevNode = curNode;

		/*Now decPtr points to <space> or '/' or '>'*/
		cx_dec_dbg ("node: %s\n", curNode->tagField);
	} else {
		if ((size_t)(decPtr - cookie->xs) == strlen (cookie->xs)) {
			cx_dec_dbg ("DONE!!\n");
			return CX_SUCCESS;
		}
		/*we have a content of parent now.. not a child*/
		status = getNodeFromNewTag (prevNode, &curNode, CXN_CONTENT, &decPtr);
		if (CX_SUCCESS != status) {
				printf ("content node addition failed\n");
				return status;
		}
		cx_dec_dbg ("content: %s\n", curNode->tagField);
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
		cx_dec_dbg ("self tag: %s\n", curNode->tagField);
		goto SELF_TAG;
	} else if ((curNode->nodeType == CXN_PARENT)) {
#if CX_USING_TAG_ATTR
		/*if we have attributes, get them*/
		status = getNodeAttr (curNode, &decPtr);
		if (CX_SUCCESS != status) {
			printf ("Erroneous attribute attempted!\n");
			return status;
		}
#endif
	}

	if (*decPtr == '/') {
SELF_TAG:
		/*might be a self-ending-tag*/
		if (*(decPtr+1) !='>') {
			printf ("Invalid Tag End for %s\n", curNode->tagField);
			return CX_ERR_BAD_TAG;
		}
		decPtr++;
		curNode->nodeType = CXN_SINGLE;
		cx_dec_dbg ("single node: %s\n", curNode->tagField);
	}
	
	if (*decPtr == '>') {
		decPtr++;
	}

	goto NEW_TAG;
}

cx_status_t cx_DecPkt (void **_cookie, char *str)
{
	cx_status_t xStatus;
	cx_cookie_t *cookie;
	if (!str || (strlen (str) >= CX_MAX_DEC_STR_SZ)) {
		printf ("Invalid packet\n");
		return CX_FAILURE;
	}

	if ((*str != '<') && !(str = strchr (str, '<'))) {
		printf ("No XML start-tag\n");
		return CX_FAILURE;
	}

	_cx_calloc (cookie, sizeof (*cookie));
	cx_null_lfail (cookie);

	cookie->xs = str;

	if (CX_SUCCESS != cx_BuildTreeFromXmlString (cookie)) {
		return CX_FAILURE;
	}

CX_ERR_LBL:
	return xStatus;
}
