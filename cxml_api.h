
#ifndef __CXML_API_H
#define __CXML_API_H

#include <stdint.h>
#include <errno.h>

#include "cxml_cfg.h"

#pragma pack(1)

typedef enum {
    CX_SUCCESS = 0,

	/*Buffer/Pointer errors*/
    CX_ERR_ENC_OVERFLOW,
    CX_ERR_DEC_OVERFLOW,
    CX_ERR_ALLOC,
    CX_ERR_NULL_PTR,

	/*Node errors*/
	CX_ERR_INVALID_NEW_NODE,
	CX_ERR_INVALID_NODE,
	CX_ERR_INVALID_ROOT,
	CX_ERR_NULL_NODENAME,
	CX_ERR_LONE_ROOT,
	CX_ERR_NODE_NOT_FOUND,
	CX_ERR_ROOT_FILLED,
	CX_ERR_ESTRANGED_NODE,
	CX_ERR_NEXT_NODE_FILLED,

	/*Attr errors*/
	CX_ERR_INVALID_ATTR,
	CX_ERR_NULL_ATTRNAME,
	CX_ERR_NULL_ATTRVALUE,

	/*Tag errors*/
    CX_ERR_INVALID_TAG,
	CX_ERR_LONE_TAG,
	CX_ERR_UNCLOSED_TAG,
	CX_ERR_CLOSED_TAG_MISMATCH,

	/*XML string wide errors*/
    CX_ERR_INVALID_XML,

	/*Unidentified errors*/
    CX_FAILURE,
} cx_status_t;

typedef enum {
    CXN_MIN = 0,
    CXN_PARENT,
    CXN_SINGLE,
    CXN_COMMENT,
    CXN_INSTR,
    CXN_CDATA,
    CXN_CONTENT,
    CXN_MAX,
} cxn_type_t;

typedef enum {
	/* Is the order changes here, take care of doing
	 * corresponding changes in IS_INVALID_ATTR_TYPE() */
    CXATTR_STR,
    CXATTR_CHAR,
    CXATTR_UI8,
    CXATTR_SI8,
    CXATTR_UI16,
    CXATTR_SI16,
    CXATTR_UI32,
    CXATTR_SI32,
    CXATTR_FLOAT,
    CXATTR_MAX,
} cxattr_type_t;
#define IS_INVALID_ATTR_TYPE(type) \
	((type < CXATTR_STR) || (type > CXATTR_FLOAT))

typedef enum {
    CXADD_MINTYPE,
    CXADD_CHILD,
    CXADD_NEXT,
    CXADD_FIRST,
    CXADD_MAXTYPE,
} cx_Addtype_t;

typedef union attrValue_union {
    char      *str;
    char      ch;
    int8_t    n_i8;
    uint8_t   n_u8;
    uint16_t  n_u16;
    int16_t   n_i16;
    uint32_t  n_u32;
    int32_t   n_i32;
    float     f;
    uint32_t  xVal; /*to refer all variables here in generic*/
} cxa_value_u;

/**
 * @func   : cx_EncPkt
 * @brief  : build an xml formatted string from an existing xml-tree
 * @called : when an xml tree is finalised and xml string is needed
 *           to be built from that tree
 * @input  : void *_cookie - pointer to select xml-context
 * @output : char **xmlData - pointer to store the final xml string 
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
cx_status_t cx_EncPkt (void *_cookie, char **xmlData);

/**
 * @func   : cx_DecPkt
 * @brief  : setup a cookie and build an xml tree from an existing xml-string
 * @called : when a peer sends xml content packet and we need to
 *           validate and parse the same properly to build xml-tree
 * @input  : char *str - existing xml string
 *           char *name - name of this decoding session cookie 
 * @output : void **_cookie - pointer filled to the freshly created xml-context
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
cx_status_t cx_DecPkt (void **_cookie, char *str, char *name);

/**
 * @func   : cx_AddFirstNode
 * @brief  : adds first(root?) node to tree
 * @called : when populating tree for the first time
 * @input  : void *_cookie - pointer to select xml-context
 *           char *name - tagField
 *           cxn_type_t nodeType - type of node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddFirstNode(_cookie, name, nodeType) \
    _cx_AddNode (_cookie, name, nodeType, NULL, CXADD_FIRST)

/**
 * @func   : cx_AddParentNode
 * @brief  : adds PARENT type node to tree
 * @called : when populating tree with a PARENT type node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *name - tagField
 *           char *addTo - name of node to which current node is added
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddParentNode(_cookie, name, addTo, addType) \
    _cx_AddNode (_cookie, name, CXN_PARENT, addTo, addType)

/**
 * @func   : cx_AddSingleNode
 * @brief  : adds SINGLE type node to tree
 * @called : when populating tree with a SINGLE type node (tag is '<node/>')
 * @input  : void *_cookie - pointer to select xml-context
 *           char *name - tagField
 *           char *addTo - name of node to which current node is added
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddSingleNode(_cookie, name, addTo, addType) \
    _cx_AddNode (_cookie, name, CXN_SINGLE, addTo, addType)

#if CX_USING_COMMENTS
/**
 * @func   : cx_AddCommentNode
 * @brief  : adds COMMENT type node to tree
 * @called : when populating tree with a COMMENT type node: <!-- comment -->
 * @input  : void *_cookie - pointer to select xml-context
 *           char *comment - comment string
 *           char *addTo - name of node to which current node is added
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddCommentNode(_cookie, comment, addTo, addType) \
    _cx_AddNode (_cookie, comment, CXN_COMMENT, addTo, addType)
#endif

#if CX_USING_CDATA
/**
 * @func   : cx_AddCDataNode
 * @brief  : adds CDATA type node to tree
 * @called : when populating tree with a CDATA type node as below-
 *           <![CDATA[ cdata-string ]]>
 * @input  : void *_cookie - pointer to select xml-context
 *           char *CData - CDATA string
 *           char *addTo - name of node to which current node is added
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddCDataNode(_cookie, CData, addTo, addType) \
    _cx_AddNode (_cookie, CData, CXN_CDATA, addTo, addType)
#endif

#if CX_USING_INSTR
/**
 * @func   : cx_AddInstrNode
 * @brief  : adds INSTRUCTION type node to tree: <?instr-string?>
 * @called : when populating tree with a INSTRUCTION type node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *instr - instruction string
 *           char *addTo - name of node to which current node is added
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddInstrNode(_cookie, instr, addTo, addType) \
    _cx_AddNode (_cookie, instr, CXN_INSTR, addTo, addType)
#endif

/**
 * @func   : cx_AddContentNode
 * @brief  : adds CONTENT type node to tree
 * @called : when populating tree with a CONTENT type node(just data, no tag)
 * @input  : void *_cookie - pointer to select xml-context
 *           const char *new - tag field name
 *           cxn_type_t nodeType - Type of node
 *           char *addTo - name of node to which current node is added
 *           cx_Addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddContentNode(_cookie, content, addTo, addType) \
    _cx_AddNode (_cookie, content, CXN_CONTENT, addTo, addType)

cx_status_t _cx_AddNode (void *_cookie, const char *new, cxn_type_t nodeType, const char *addTo, cx_Addtype_t addType);

/**
 * @func   : cx_AddAttr_CHAR
 * @brief  : adds CHAR type attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_CHAR(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_CHAR, node)

/**
 * @func   : cx_AddAttr_STR
 * @brief  : adds STRING type attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_STR(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)attrValue, CXATTR_STR, node)

/**
 * @func   : cx_AddAttr_ui8
 * @brief  : adds unsigned 8-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_ui8(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_UI8, node)

/**
 * @func   : cx_AddAttr_si8
 * @brief  : adds signed 8-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_si8(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_SI8, node)

/**
 * @func   : cx_AddAttr_ui16
 * @brief  : adds unsigned 16-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_ui16(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_UI16, node)

/**
 * @func   : cx_AddAttr_si16
 * @brief  : adds signed 16-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_si16(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_SI16, node)

/**
 * @func   : cx_AddAttr_ui32
 * @brief  : adds unsigned 32-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_ui32(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_UI32, node)

/**
 * @func   : cx_AddAttr_si32
 * @brief  : adds signed 32-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_si32(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_SI32, node)

/**
 * @func   : cx_AddAttr_float
 * @brief  : adds float type attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : void *_cookie - pointer to select xml-context
 *           char *attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
#define cx_AddAttr_float(_cookie, attrname, attrValue, node) \
    _cx_AddAttrToNode (_cookie, attrname, \
			(cxa_value_u *)&attrValue, CXATTR_FLOAT, node)

#if CX_USING_TAG_ATTR
cx_status_t _cx_AddAttrToNode (void *_cookie, char *attrName, cxa_value_u *value, cxattr_type_t type, char *node);
#endif /*CX_USING_TAG_ATTR*/

/**
 * @func   : cx_CreateSession
 * @brief  : Create new session for xml operations and give out session cookie
 * @called : when new xml encoding session is required, NOT FOR DECODER SESSION
 * @input  : char *name - name identifying the purpose of this session
 *           char *uxs - optional string pointer for encoder to from xml string
 *           initXmlLength - Iff uxs is valid, gives an initial length
 * @output : void *_cookie - pointer to xml-context after proper session setup
 * @return : CX_SUCCESS on success
 *           non-zero value indicating type of failure
 */
cx_status_t cx_CreateSession (void **_cookie, char *name, char *uxs, uint32_t initXmlLength);

/**
 * @func   : cx_DestroySession
 * @brief  : Destroy an existing session
 * @called : when particular xml session is no more required
 * @input  : void *_cookie - pointer to a valid xml-context
 * @output : none
 * @return : void
 */
void cx_DestroySession (void *_cookie);

/**
 * @func   : cx_strerr
 * @brief  : returns a string describing a cx_status_t type value
 * @called : Mostly when application needs to understand/log cxml errors
 * @input  : cx_status_t cx_st - cxml library returned status
 * @output : none
 * @return : pointer to string describing the error type
 */
const char *cx_strerr (cx_status_t cx_st);

#endif /*__CXML_API_H*/
