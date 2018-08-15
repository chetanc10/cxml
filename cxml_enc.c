#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cxml.h"
#include "cxml_api.h"

#if CX_ENC_DBG_EN
#define cx_enc_dbg printf
#else
#define cx_enc_dbg(...)
#endif

#define IS_NODE_TYPE_VALID(nodeType) \
	((nodeType > CXN_MIN) && (nodeType < CXN_MAX))

#define IS_INVALID_NODE_TYPE(type) (!IS_NODE_TYPE_VALID(type))

#define IS_NODE_SINGLE_VALID(node) \
	((node->nodeType != CXN_SINGLE) || !(node->children))

#if CX_USING_TAG_ATTR
#define IS_HAVING_ATTR(node) (node->numOfAttr && node->attrList)
#endif

cx_node_t *rootNode = NULL;
static char *encodedPktStr;

#if CX_USING_TAG_ATTR
static cx_status_t putNodeAttr(cx_node_t *xmlNode, char **encPtr)
{
	uint8_t n = xmlNode->numOfAttr;
	cxn_addr_t *attrListPtr = xmlNode->attrList;

	for(; n && attrListPtr; n--, attrListPtr = attrListPtr->next) {
		*encPtr += sprintf(*encPtr, " %s=\"%s\"", \
				attrListPtr->attrName, attrListPtr->attrValue);
		cx_enc_dbg ("attr::\n\r%s\n\r", encodedPktStr);
	}

	return (n || attrListPtr) ? CX_ERR_BAD_ATTR:CX_SUCCESS;
}
#endif

static cx_status_t enc_buildXmlBuf(char **encPtr)
{
	cx_node_t *curNode = rootNode;

NEW_NODE:
	if(curNode->nodeType != CXN_CONTENT) {
		*encPtr += sprintf(*encPtr, "<");
	}

	switch(curNode->nodeType) {
#if CX_USING_COMMENTS
		case CXN_COMMENT: *encPtr += sprintf(*encPtr, "!--"); break;
#endif
#if CX_USING_CDATA
		case CXN_CDATA: *encPtr += sprintf(*encPtr, "![CDATA["); break;
#endif
#if CX_USING_INSTR
		case CXN_INSTR: *encPtr += sprintf(*encPtr, "?"); break;
#endif
		default: break;
	}

	*encPtr += sprintf(*encPtr, "%s", curNode->tagField);
	cx_enc_dbg ("1:\n\r%s\n\r", encodedPktStr);

	switch(curNode->nodeType) {
		case CXN_SINGLE:
		case CXN_PARENT:
#if CX_USING_TAG_ATTR
			if(IS_HAVING_ATTR(curNode) && \
					(CX_SUCCESS != putNodeAttr(curNode, encPtr))) {
				return CX_FAILURE;
			}
#endif
			*encPtr += sprintf(*encPtr, \
					(curNode->nodeType == CXN_SINGLE)?"/>":">");
			break;
#if CX_USING_COMMENTS
		case CXN_COMMENT:
			*encPtr += sprintf(*encPtr, "-->");
			break;
#endif
#if CX_USING_CDATA
		case CXN_CDATA:
			*encPtr += sprintf(*encPtr, "]]>");
			break;
#endif
#if CX_USING_INSTR
		case CXN_INSTR:
			*encPtr += sprintf(*encPtr, "?>");
			break;
#endif
		default:
			break;
	}

	cx_enc_dbg ("2:\n\r%s\n\r", encodedPktStr);

	if(((*encPtr - encodedPktStr) > MAX_TX_XML_PKT_SIZE)) {
		printf("OverFlown xml encoded tree for buffer\n");
		return CX_ERR_ENC_OVERFLOW;
	}

	if(curNode->children) {
		curNode = curNode->children;
		goto NEW_NODE;
	}

NEXT_NODE:
	if(curNode->next) {
		if(curNode->nodeType == CXN_PARENT) {
			*encPtr += sprintf(*encPtr, "</%s>", curNode->tagField);
		}
		curNode = curNode->next;
		cx_enc_dbg ("3:\n\r%s\n\r", encodedPktStr);
		goto NEW_NODE;
	} else {
		if(curNode->nodeType == CXN_PARENT) {
			*encPtr += sprintf(*encPtr, "</%s>", curNode->tagField);
			cx_enc_dbg ("4:\n\r%s\n\r", encodedPktStr);
		}
		if(curNode->parent) {
			curNode = curNode->parent;
			cx_enc_dbg ("back to: %s\n\r", curNode->tagField);
			goto NEXT_NODE;
		}
	}

	return CX_SUCCESS;
}

cx_status_t encode_xml_pkt(char **xmlData)
{
    cx_status_t ret;
	char *encPtr = NULL;

	if((NULL == rootNode) || \
			IS_INVALID_NODE_TYPE(rootNode->nodeType) || \
			!IS_NODE_SINGLE_VALID(rootNode)) {
		return CX_FAILURE;
	}

	encodedPktStr = malloc(MAX_TX_XML_PKT_SIZE);
	if(!encodedPktStr) {
		printf("Error allocating in XML encoder\n");
		return CX_ERR_ALLOC;
	}

	memset(encodedPktStr, 0, MAX_TX_XML_PKT_SIZE);
	encPtr = encodedPktStr;

	ret = enc_buildXmlBuf(&encPtr);
	if(CX_SUCCESS != ret) {
		printf("failed to build xml packet buffer!\n");
		_cx_free(encodedPktStr);
	    cx_destroyTree();
		return CX_FAILURE;
	}

	*xmlData = encodedPktStr;
	cx_enc_dbg ("PACKET: \n%s\n", encodedPktStr);
	return ret;
}

#if CX_USING_TAG_ATTR
static void destroyAttrList(cxn_addr_t **list)
{
	cxn_addr_t *cur = *list, *prev;

	do {
		prev = cur;
		_cx_free(cur->attrName);
		_cx_free(cur->attrValue);
	} while(NULL != (cur = prev->next));
}
#endif

void cx_destroyTree(void)
{
	cx_node_t *curNode = rootNode;

	if(!curNode) {
		return;
	}

	cx_enc_dbg ("Destroying xml tree from: %s\r\n", rootNode->tagField);

	rootNode = NULL;
	while(curNode) {
		cx_enc_dbg ("cur: %p:%s\n", curNode, curNode->tagField);
		if(curNode->children) {
			curNode = curNode->children;
		} else {
			cx_node_t *temp;
			uint8_t isParent = 0;
FREE_NODE:
			if(curNode->next) {
				temp = curNode->next;
			} else {
				temp = curNode->parent;
				if(temp) {
					isParent = 1;
				}
			}
			cx_enc_dbg ("freeing: %s\n", curNode->tagField);
			_cx_free(curNode->tagField);
#if CX_USING_TAG_ATTR
			if(curNode->attrList) {
				destroyAttrList(&(curNode->attrList));
			}
#endif
			cx_enc_dbg ("freeing: %p\r\n", curNode);
			_cx_free(curNode);
			if(isParent) { /*all children dead, so parent also dies*/
				curNode = temp;
				temp = temp->next;
				isParent = 0;
				goto FREE_NODE;
			}
			curNode = temp;
		}
	}
	cx_enc_dbg ("destruction of the Tree complete\r\n");
}

cx_node_t *cx_findNodeWithTag(char *name, cx_node_t *start)
{
	cx_node_t *curNode = start;

	if(!start) {
		return (cx_node_t *)NULL;
	}

	while(curNode) {
 	    cx_enc_dbg ("check %s\n", curNode->tagField);
		if(!strcmp(name, curNode->tagField))
			break;
		if(curNode->children) {
			curNode = curNode->children;
		} else { /*try list of children*/
			/*If no more children, goto next node of parent, 
			 *since parent is already checked*/
			curNode = (curNode->next)?curNode->next:curNode->parent->next;
		}
	}

 	cx_enc_dbg ("findNode: %s %s\r\n", name, curNode?"success":"failed");

	return curNode;
}

#if CX_USING_TAG_ATTR
cx_status_t _cx_addAttrToNode(char *attrName, cxa_value_u *value, cxa_type_t type, char *nodeName)
{
#define IS_INVALID_ATTR_TYPE(type) ((type < CXA_CHAR) || (type > CXA_MAX))

	_cx_def_fmts_array(fmt_spec);
	cx_node_t *node;
	cxn_addr_t *newAttr;
	char *temp;
	uint8_t n = 0;

	if(!nodeName) {
		printf("null address given for node!\n");
		return CX_ERR_NULL_MEM;
	}
	if(!attrName || !value || IS_INVALID_ATTR_TYPE(type)) {
		printf("No proper attr input\n");
		return CX_ERR_BAD_ATTR;
	}

	node = cx_findNodeWithTag(nodeName, rootNode);
	if(!node) {
		printf("No node named %s\n", nodeName);
		return CX_ERR_BAD_NODE;
	}

	_cx_calloc (newAttr, sizeof (cxn_addr_t));
	if(!newAttr) {
		printf("memory allocation error for new attr\n");
		return CX_ERR_ALLOC;
	}

	newAttr->attrName = _cx_strndup(attrName, strlen(attrName), attrName);
	cx_enc_dbg ("attr: %s=", newAttr->attrName);
	_cx_calloc (temp, 2+20);
	if(!temp) {
		printf("memory allocation error for new attr value\n");
		return CX_ERR_ALLOC;
	}

	cx_enc_dbg ("val: %u\n", value->n_u8);
	n += sprintf(temp + n,fmt_spec[type], value->xVal);
	newAttr->attrValue = _cx_strndup(temp, strlen(temp), temp);
	_cx_free(temp);
	cx_enc_dbg ("%s\n", newAttr->attrValue);

	if(!node->attrList) {
		cx_enc_dbg ("1st attr\n");
		node->attrList = newAttr;
		node->numOfAttr = 0;
	} else {
		cxn_addr_t *link = node->attrList;
		cx_enc_dbg ("linking attr\n");
		for(; link->next; link = link->next);
		link->next = newAttr;
	}

	node->numOfAttr++;

	return CX_SUCCESS;
}
#endif

/**
 * @func   : addNodeToTree
 * @brief  : adds a new node to tree
 * @called : when populating tree with a new node as child/next/first
 * @input  : char *new - tagField for tagName/content/cdata/comment
 *           char *addTo - tagField for previous node to which new one is added
 *                       This is required so that user can properly populate nodes
 *           cxn_type_t nodeType - type of new node
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
cx_status_t addNodeToTree(const char *new, cxn_type_t nodeType, const char *addTo, cx_addtype_t addType)
{
	/*give CX_SUCCESS if addtype is valid*/
#define BAD_ADDTYPE_VAL(type) \
	((type <= CX_ADD_MINTYPE) || (type >= CX_ADD_MAXTYPE))
#define INVALID_POPULATING(type) \
	((type != CX_ADD_AS_CHILD) && (type != CX_ADD_AS_NEXT))
#define ROOT_EXISTS() (NULL != rootNode)

	cx_node_t *newNode = NULL;
	static cx_node_t *prevNode = NULL;
	cx_status_t xStatus = CX_SUCCESS;

	_cx_lassert(IS_INVALID_NODE_TYPE(nodeType), CX_ERR_BAD_NODE, "Invalid nodeType\n");

	_cx_null_lassert(new);

	_cx_lassert(BAD_ADDTYPE_VAL(addType), CX_ERR_BAD_NODE, "Invalid addType\n");

	cx_enc_dbg ("newNode: %s\r\n", new);
// 	_cx_lassert((INVALID_POPULATING(addType) && ROOT_EXISTS()), CX_ERR_BAD_NODE, "Invalid addType\n");

	_cx_calloc (newNode, sizeof (cx_node_t));
	_cx_null_lassert(newNode);

	newNode->tagField = _cx_strndup((char *)new, strlen(new), (char *)new);
	_cx_null_lassert(newNode->tagField);

	newNode->nodeType = nodeType;

	if(addType == CX_ADD_AS_FIRST) {
		/*destroy any previous tree to have new tree planted*/
	    cx_destroyTree();
		rootNode = prevNode = newNode;
		cx_enc_dbg ("\"%s\" is rootNode\n", rootNode->tagField);
		return CX_SUCCESS;
	} else {
		_cx_null_lassert(addTo);
	}

	if(CX_SUCCESS != strcmp(addTo, prevNode->tagField)) {
	   	if(CX_SUCCESS == strcmp(addTo, prevNode->parent->tagField)) {
			cx_enc_dbg ("adding %s to a prev parent: %s\n", new, prevNode->parent->tagField);
			prevNode = prevNode->parent;
		} else {
			printf("don't know :%s..%s..%s\n", addTo, new, prevNode->tagField);
			xStatus = CX_ERR_BAD_NODE;
			goto CX_ERR;
		}
	}

	if (addType == CX_ADD_AS_CHILD) {
		if(prevNode->children) {
			printf("%s has already got a child\n", addTo);
			xStatus = CX_ERR_BAD_NODE;
			goto CX_ERR;
		} 
		cx_enc_dbg ("adding %s as child to %s\n", new, addTo);
		prevNode->children = newNode;
		prevNode->numOfChildren = 1;
		newNode->parent = prevNode;
	} else {
		if(prevNode->next) {
			printf("%s has already got a node next to it!\n", prevNode->tagField);
			xStatus = CX_ERR_BAD_NODE;
			goto CX_ERR;
		}
		cx_enc_dbg ("adding as next\n");
		newNode->parent = prevNode->parent;
		prevNode->next = newNode;
		prevNode->parent->numOfChildren++;
	}

	prevNode = newNode;

CX_ERR:
	if(xStatus != CX_SUCCESS) {
		if(newNode->tagField)
			_cx_free(newNode->tagField);
		if(newNode)
			_cx_free(newNode);
		cx_destroyTree();
		prevNode = NULL;
	}
	cx_enc_dbg ("now prev: %s\n", prevNode->tagField);
	return xStatus;
}
