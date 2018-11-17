#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cxml.h"
#include "cxml_api.h"
#include "cxml_errchk.h"

#define IS_NODE_TYPE_VALID(nodeType) \
	((nodeType >= CXN_PARENT) && (nodeType < CXN_MAX))

#define IS_INVALID_NODE_TYPE(type) (!IS_NODE_TYPE_VALID(type))

#define IS_ROOTNODE_SINGLE(node) \
	((node->nodeType != CXN_SINGLE) || !(node->children))

#if CX_USING_TAG_ATTR

#define IS_HAVING_ATTR(node) (node->numOfAttr && node->attrList)

static void _cx_PutNodeAttr (cx_node_t *xmlNode, char **encPtr)
{ /*Remove numOfAttr and add last pointer -TODO*/
	uint8_t n = xmlNode->numOfAttr;
	cxn_attr_t *attrListPtr = xmlNode->attrList;

	for (; n && attrListPtr; n--, attrListPtr = attrListPtr->next) {
		*encPtr += sprintf (*encPtr, " %s=\"%s\"", \
				attrListPtr->attrName, attrListPtr->attrValue);
		cx_enc_dbg ("attr::\n\r%s\n\r", *encPtr);
	}
}

#endif

static void _xml_verstring (char **encPtr)
{
	*encPtr += sprintf (*encPtr, "<?%s?>", XML_INSTR_STR);
}

static cx_status_t cx_BuildXmlString (cx_cookie_t *cookie)
{
	/* _cxe_fmt Locked in correspondence with cxn_type_t,
	 * _cxe_fmt[0] contains beginning of node data
	 * _cxe_fmt[1] contains the end! */
	const char *_cxe_fmt[2][CXN_MAX] = {
		[0] = {
			[CXN_PARENT]    = "<%s",
			[CXN_SINGLE]    = "<%s",
			[CXN_COMMENT]   = "<!--%s",
			[CXN_INSTR]     = "<?%s?>",
			[CXN_CDATA]     = "<![CDATA[",
			[CXN_CONTENT]   = "%s",
		},
		[1] = {
			[CXN_PARENT]    = ">",
			[CXN_SINGLE]    = "/>",
			[CXN_COMMENT]   = "-->",
			[CXN_INSTR]     = "?>",
			[CXN_CDATA]     = "]]>",
			[CXN_CONTENT]   = "",
		},
	};
	cx_node_t *curNode = cookie->root;
	char *encPtr = cookie->xs;

	_xml_verstring (&encPtr);

	while (1) {
		encPtr += sprintf (encPtr, \
				_cxe_fmt[0][curNode->nodeType], curNode->tagField);

#if CX_USING_TAG_ATTR
		if (IS_HAVING_ATTR (curNode)) {
			_cx_PutNodeAttr (curNode, &encPtr);
		}
#endif

		strcat (encPtr, _cxe_fmt[1][curNode->nodeType]);
		encPtr += strlen (_cxe_fmt[1][curNode->nodeType]);

		cx_enc_dbg ("++\n\r%s\n\r..", cookie->xs);

		cx_rfail (((encPtr - cookie->xs) > CX_MAX_ENC_STR_SZ), \
				CX_ERR_ENC_OVERFLOW);

		if (curNode->children) {
			curNode = curNode->children;
			continue;
		}

NEXT_NODE:
		if (curNode->nodeType == CXN_PARENT) {
			encPtr += sprintf (encPtr, "</%s>", curNode->tagField);
			cx_enc_dbg ("+++\n\r%s\n\r...", cookie->xs);
			cx_rfail (((encPtr - cookie->xs) > CX_MAX_ENC_STR_SZ), \
					CX_ERR_ENC_OVERFLOW);
		}
		if (curNode->next) {
			curNode = curNode->next;
		} else if (curNode->parent) {
			curNode = curNode->parent;
			cx_enc_dbg ("back to: %s\n\r", curNode->tagField);
			goto NEXT_NODE;
		} else {
			break;
		}
	}

	return CX_SUCCESS;
}

cx_status_t cx_EncPkt (void *_cookie, char **xmlData)
{
    cx_status_t xStatus;
	cx_cookie_t *cookie = (cx_cookie_t *)_cookie;

	cx_null_rfail (cookie->root);
	cx_rfail (IS_INVALID_NODE_TYPE(cookie->root->nodeType), \
			CX_ERR_INVALID_ROOT);
	cx_rfail (!IS_ROOTNODE_SINGLE (cookie->root), CX_ERR_LONE_ROOT);

	if (!cookie->xsIsFromUser) { /*if we have to manage xml-string memory*/
		/*get actual xml-strlen including tag-delimiters! -TODO*/
		_cx_calloc (cookie->xs, cookie->xmlLength);
		cx_alloc_rfail (cookie->xs);
	}

	xStatus = cx_BuildXmlString (cookie);
	cx_rfail ((xStatus != CX_SUCCESS), xStatus);

	if (!cookie->xsIsFromUser) {
		cx_null_rfail (xmlData);
		*xmlData = cookie->xs;
	}

	cx_enc_dbg ("PACKET: \n%s\n", cookie->xs);

	return xStatus;
}

#if CX_USING_TAG_ATTR
cx_status_t _cx_AddAttrToNode (void *_cookie, char *attrName, cxa_value_u *value, cxattr_type_t type, char *nodeName)
{
	cx_status_t xStatus;
	_cx_def_fmts_array (fmt_spec);
	cx_cookie_t *cookie = (cx_cookie_t *)_cookie;
	cx_node_t *node;
	cxn_attr_t *newAttr;

	cx_rfail (!nodeName, CX_ERR_NULL_NODENAME);
	cx_rfail (!attrName, CX_ERR_NULL_ATTRNAME);
	cx_rfail (!value, CX_ERR_NULL_ATTRVALUE);
	cx_rfail (IS_INVALID_ATTR_TYPE (type), CX_ERR_INVALID_ATTR);

	/* Try to update this to force user to follow xml string update in order.
	 * adding child to nodeX, then adding attr to nodeX is time-wasting -TODO*/
	node = cx_FindNodeWithTag (cookie, nodeName);
	cx_rfail (!node, CX_ERR_NODE_NOT_FOUND);

	_cx_malloc (newAttr, sizeof (cxn_attr_t));
	cx_alloc_rfail (newAttr);

	newAttr->attrName = _cx_strndup (attrName, strlen (attrName), attrName);
	cx_lfail (!newAttr->attrName, CX_ERR_ALLOC);
	cx_enc_dbg ("attr: %s=", newAttr->attrName);
	{
		/* If value's a user-string, allocate memory to suit it's length
		 * If not a string type, get the size using the type*/
		_cx_def_size_array (size_spec);
		size_t _sz = (type == CXATTR_STR) ? \
					 strlen ((char *)value) : size_spec[type];
		_cx_calloc (newAttr->attrValue, 2/*2 " symbols*/ + _sz);
		cx_alloc_lfail (newAttr->attrValue);
	}
	newAttr->next = NULL;

	switch (type) {
		case CXATTR_STR:
			sprintf (newAttr->attrValue, fmt_spec[type], (char *)value);
			break;
		case CXATTR_CHAR:
			sprintf (newAttr->attrValue, fmt_spec[type], (char)value->xVal);
			break;
		case CXATTR_UI8:
			sprintf (newAttr->attrValue, fmt_spec[type], (uint8_t)value->xVal);
			break;
		case CXATTR_SI8:
			sprintf (newAttr->attrValue, fmt_spec[type], (int8_t)value->xVal);
			break;
		case CXATTR_UI16:
			sprintf (newAttr->attrValue, fmt_spec[type], (uint16_t)value->xVal);
			break;
		case CXATTR_SI16:
			sprintf (newAttr->attrValue, fmt_spec[type], (int16_t)value->xVal);
			break;
		case CXATTR_UI32:
			sprintf (newAttr->attrValue, fmt_spec[type], (uint32_t)value->xVal);
			break;
		case CXATTR_SI32:
			sprintf (newAttr->attrValue, fmt_spec[type], (int32_t)value->xVal);
			break;
		case CXATTR_FLOAT:
			sprintf (newAttr->attrValue, fmt_spec[type], (float)value->xVal);
			break;
		case CXATTR_MAX: /*Just removing compiler warning*/
			break;
	}
	cx_enc_dbg ("attr-val-str: %s\n", newAttr->attrValue);

	if (!node->attrList) {
		cx_enc_dbg ("1st attr\n");
		node->attrList = newAttr;
		node->numOfAttr = 0;
	} else {
		cxn_attr_t *link = node->attrList;
		cx_enc_dbg ("linking attr\n");
		for (; link->next; link = link->next);
		link->next = newAttr;
	}

	node->numOfAttr++;

	return CX_SUCCESS;

CX_ERR_LBL:
	_cx_free (newAttr->attrName);
	_cx_free (newAttr->attrValue);
	return xStatus;
}
#endif

/**
 * @func   : _cx_AddNode
 * @brief  : adds a new node to tree
 * @called : when populating tree with a new node as child/next/first
 * @input  : char *new - tagField for tagName/content/cdata/comment
 *           char *addTo - tagField for previous node to which new one is added
 *                       Required so that user can properly populate nodes
 *           cxn_type_t nodeType - type of new node
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/
 * 			 CX_ERR_NULL_PTR/CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
cx_status_t _cx_AddNode (void *_cookie, const char *new, cxn_type_t nodeType, const char *addTo, cx_Addtype_t addType)
{
	/*give CX_SUCCESS if addtype is valid*/
#define BAD_ADDTYPE_VAL(type) \
	((type <= CXADD_MINTYPE) || (type >= CXADD_MAXTYPE))
#define INVALID_POPULATING(type) \
	((type != CXADD_CHILD) && (type != CXADD_NEXT))

	cx_node_t *newNode = NULL;
	cx_cookie_t *cookie = (cx_cookie_t *)_cookie;
	cx_node_t *prevNode;
	cx_status_t xStatus = CX_SUCCESS;

	cx_rfail (IS_INVALID_NODE_TYPE(nodeType), CX_ERR_INVALID_NODE);

	cx_null_lfail (new);

	cx_rfail (BAD_ADDTYPE_VAL(addType), CX_ERR_INVALID_NEW_NODE);

	cx_enc_dbg ("newNode: %s\r\n", new);

	_cx_calloc (newNode, sizeof (cx_node_t));
	cx_alloc_rfail (newNode);

	newNode->tagField = _cx_strndup ((char *)new, strlen (new), (char *)new);
	cx_alloc_lfail (newNode->tagField);

	newNode->nodeType = nodeType;

	if (addType == CXADD_FIRST) {
		cx_lfail ((cookie->root != NULL), CX_ERR_ROOT_FILLED);
		/*Xml Origins: root-node*/
		cookie->root = cookie->recent = newNode;
		cx_enc_dbg ("\"%s\" is root-node\n", newNode->tagField);
		return CX_SUCCESS;
	}

	/*If not first node, addTo is expected to be valid pointer*/
	cx_null_lfail (addTo);

	prevNode = cookie->recent;

	if (CX_SUCCESS != strcmp (addTo, prevNode->tagField)) {
	   	if (CX_SUCCESS == strcmp (addTo, prevNode->parent->tagField)) {
			cx_enc_dbg ("adding %s to a prev parent: %s\n", \
					new, prevNode->parent->tagField);
			prevNode = prevNode->parent;
		} else { /*just don't use cx_lfail API, current is good to debug*/
			cx_enc_dbg ("don't know :%s..%s..%s\n", \
					addTo, new, prevNode->tagField);
			xStatus = CX_ERR_ESTRANGED_NODE;
			goto CX_ERR_LBL;
		}
	}

	if (addType == CXADD_CHILD) {
		/*If asked to be added as child, add it to child list my mother!*/
		cx_enc_dbg ("adding %s as child to %s\n", new, addTo);
		if (prevNode->children) {
			/*Add the new born as the last one*/
			prevNode->lastChild->next = newNode;
		} else {
			/*New born is the first born */
			prevNode->children = newNode;
		}
		prevNode->lastChild = newNode;
		newNode->parent = prevNode;
	} else {
		if (prevNode->next) {
			xStatus = CX_ERR_NEXT_NODE_FILLED;
			goto CX_ERR_LBL;
		}
		cx_enc_dbg ("adding %s next to %s\n", new, addTo);
		newNode->parent = prevNode->parent;
		prevNode->next = newNode;
	}

	cookie->recent = newNode;

CX_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		_cx_free (newNode->tagField);
		_cx_free (newNode);
	} else { /*No failure*/
		cx_enc_dbg ("now prev: %s\n", newNode->tagField);
	}
	return xStatus;
}

