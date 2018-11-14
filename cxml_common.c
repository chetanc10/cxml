
#include <stdio.h>

#include "cxml.h"
#include "cxml_api.h"
#include "cxml_errchk.h"

#if CX_COM_DBG_EN
#define cx_com_dbg printf
#else
#define cx_com_dbg(...)
#endif

/*NOTE: Refer/Update according to cx_status_t definition!*/
const char *cx_ErrStr[] = {
	"Success",

	/*Buffer/Pointer errors*/
	"Encoder Overflow",
	"Decoder Overflow",
	"Memory Allocation",
	"Null Pointer",

	/*Node errors*/
	"Invalid new Node type",
	"Invalid Node",
	"Invalid Root Node",
	"Node Name Null Pointer",
	"Root Node is Single or Infertile",
	"Node Not Found",
	"Root Node already filled",
	"Node is hard to link - link-to node is way old",
	"Unable to overwrite next-node",

	/*Attr errors*/
	"Invalid Attr Type",
	"Attr Name Null Pointer",
	"Attr Value Null Pointer",

	/*Tag errors*/
	"Invalid Tag",
	"Tag is neither parent nor child!",
	"Tag is not properly closed",
	"Closed Tag doesn't have expected tag-name",

	/*XML string wide errors*/
	"Invalid/Corrupt XML string",

	/*Unidentified errors*/
	"Unknown failure",
};

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
char *_cx_strndup (char *src, size_t maxLen, char *dName)
{
	char *dest = NULL;
	size_t dLen, sLen;

	if (src && ((sLen = strlen (src)) != 0)) {

		dLen = ((sLen < maxLen) ? sLen : maxLen);

		_cx_calloc (dest, dLen + 1); /*+1 for NULL char to end-string*/

		if (dest) {
			strncpy (dest, src, dLen);
		}
	}

	return dest;
}

#if CX_USING_TAG_ATTR
static void destroyAttrList (cxn_attr_t **list)
{
	cxn_attr_t *cur = *list, *prev;

	do {
		prev = cur;
		_cx_free (cur->attrName);
		_cx_free (cur->attrValue);
		_cx_free (cur);
	} while (NULL != (cur = prev->next));
}
#endif

/**
 * @func   : _cx_destroyTree
 * @brief  : destroys a given xml tree
 * @called : when this xml tree is no longer required
 * @input  : cx_cookie_t *cookie - pointer to select xml-context
 * @output : none
 * @return : void
 */
static void _cx_destroyTree (cx_cookie_t *cookie)
{
	cx_node_t *curNode = cookie->root;

	if (!curNode) {
		return;
	}

	cx_com_dbg ("Destroying xml tree from: %s\r\n", curNode->tagField);

	while (curNode) {
		cx_com_dbg ("cur: %p:%s\n", curNode, curNode->tagField);
		if (curNode->children) {
			curNode = curNode->children;
		} else {
			cx_node_t *temp;
			uint8_t isParent = 0;
FREE_NODE:
			if (curNode->next) {
				temp = curNode->next;
			} else {
				temp = curNode->parent;
				if (temp) {
					isParent = 1;
				}
			}
			cx_com_dbg ("freeing: %s\n", curNode->tagField);
			_cx_free (curNode->tagField);
#if CX_USING_TAG_ATTR
			if (curNode->attrList) {
				destroyAttrList (&(curNode->attrList));
			}
#endif
			cx_com_dbg ("freeing: %p\r\n", curNode);
			_cx_free (curNode);
			if (isParent) { /*all children dead, so parent also dies*/
				curNode = temp;
				temp = temp->next;
				isParent = 0;
				goto FREE_NODE;
			}
			curNode = temp;
		}
	}
	cookie->root = NULL;
	cx_com_dbg ("destruction of the Tree complete\r\n");
}

cx_status_t cx_CreateSession (void **_cookie, char *name, char *uxs, uint32_t initXmlLength)
{
	cx_status_t xStatus = CX_SUCCESS;
	cx_cookie_t *cookie;

	cx_null_rfail (_cookie);
	cx_rfail ((initXmlLength >= CX_MAX_ENC_STR_SZ), CX_ERR_ENC_OVERFLOW);

	_cx_calloc (cookie, sizeof (cx_cookie_t));
	cx_alloc_lfail (cookie);

	cookie->cxCode = CX_COOKIE_MAGIC;
	strncpy (cookie->name, name ? name : "unknown", CX_COOKIE_NAMELEN);
	if (uxs != NULL) {
		cookie->uxsLength = initXmlLength;
		cookie->xsIsFromUser = 1;
		cookie->xs = uxs;
	}
	/* We could allocate a small buffer if !uxs, but do it later
	 * only when cx_EncPkt is called. If user cancels before cx_EncPkt call,
	 * it's a waste of resource to pre-allocate and free without using it.
	 * Moreover, allocating in cx_EncPkt helps us to allocate with correct
	 * length of final string required.
	 * */
	*_cookie = cookie;

CX_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		if (cookie->xsIsFromUser) {
			_cx_free (cookie->xs);
		}
		_cx_free (cookie);
	}
	return xStatus;
}

void cx_DestroySession (void *_cookie)
{
	cx_cookie_t *cookie = (cx_cookie_t *)_cookie;

	if (cookie && (cookie->cxCode == CX_COOKIE_MAGIC)) {
		if (!cookie->xsIsFromUser) {/*Library allocated xml-string? Free it!*/	
			_cx_free (cookie->xs);
		}
		_cx_destroyTree (cookie);
		cookie->cxCode = 0;
		_cx_free (cookie);
	}
}

const char *cx_strerr (cx_status_t cx_st)
{
	if ((cx_st >= 0) && (cx_st <= CX_FAILURE)) {
		return cx_ErrStr[cx_st];
	}

	return "BAD CX STATUS!";
}

