
#ifndef __CXML_API_H
#define __CXML_API_H

#include <stdint.h>
#include <errno.h>

#include "cxml_cfg.h"

/* This file gives the application interface to use cxml encoder/decoder
 * Just don't worry about symbol with '_' as first character */

#pragma pack(1)

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

typedef enum {
    CX_SUCCESS = 0,
    CX_ERR_ENC_OVERFLOW,
    CX_ERR_DEC_OVERFLOW,
    CX_ERR_ALLOC,
    CX_ERR_NULL_MEM,
    CX_ERR_BAD_NODE,
    CX_ERR_BAD_TAG,
    CX_ERR_BAD_ATTR,
    CX_ERR_BAD_NODETYPE,
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
} cx_addtype_t;

typedef struct cxn_addr_s {
    char                *attrName;
    char                *attrValue;
    struct cxn_addr_s   *next;
} cxn_addr_t;

typedef struct cx_node_s {
    uint8_t             nodeType;
    char                *tagField;
    struct cx_node_s    *parent;
#if CX_USING_TAG_ATTR
    uint8_t             numOfAttr;
    cxn_addr_t          *attrList;/*Save a tail-ptr to speedup attribute additions -TODO*/
#endif
    struct cx_node_s    *children;
    struct cx_node_s    *lastChild;
    struct cx_node_s    *next;
} __attribute__((__packed__)) cx_node_t;

/**
 * @func   : encode_xml_pkt
 * @brief  : build an xml formatted string from an existing xml-tree
 * @called : when an xml tree is finalised and xml string is needed
 *           to be built from that tree (FIXME add tree-contexts)
 * @input  : none
 * @output : char **xmlData - pointer to store the final xml string 
 * @return : CX_SUCCESS/CX_FAILURE/CX_ERR_ALLOC
 */
cx_status_t encode_xml_pkt (char **xmlData);

/**
 * @func   : decode_xml_pkt
 * @brief  : build an xml tree from an existing xml-string
 * @called : when a peer sends xml content packet and we need to
 *           validate and parse the same properly to build xml-tree
 * @input  : char *str - existing xml string
 * @output : cx_node_t **xmlNode - pointer to root-node holder
 * @return : CX_SUCCESS/CX_FAILURE/CX_ERR_ALLOC
 */
cx_status_t decode_xml_pkt (char *str, cx_node_t **xmlNode);

/**
 * @func   : cx_addFirstNode
 * @brief  : adds first(root?) node to tree
 * @called : when populating tree for the first time
 * @input  : char *name - tagField
 *           cxn_type_t nodeType - type of node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addFirstNode(name, nodeType) \
    _cx_addNode (name, nodeType, NULL, CXADD_FIRST)

/**
 * @func   : cx_addParentNode
 * @brief  : adds PARENT type node to tree
 * @called : when populating tree with a PARENT type node
 * @input  : char *name - tagField
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addParentNode(name, addTo, addType) \
    _cx_addNode (name, CXN_PARENT, addTo, addType)

/**
 * @func   : cx_addSingleNode
 * @brief  : adds SINGLE type node to tree
 * @called : when populating tree with a SINGLE type node (tag is '<node/>')
 * @input  : char *name - tagField
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addSingleNode(name, addTo, addType) \
    _cx_addNode (name, CXN_SINGLE, addTo, addType)

#if CX_USING_COMMENTS
/**
 * @func   : cx_addCommentNode
 * @brief  : adds COMMENT type node to tree
 * @called : when populating tree with a COMMENT type node: <!-- comment -->
 * @input  : char *comment - comment string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addCommentNode(comment, addTo, addType) \
    _cx_addNode (comment, CXN_COMMENT, addTo, addType)
#endif

#if CX_USING_CDATA
/**
 * @func   : cx_addCDataNode
 * @brief  : adds CDATA type node to tree
 * @called : when populating tree with a CDATA type node as below-
 *           <![CDATA[ cdata-string ]]>
 * @input  : char *CData - CDATA string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addCDataNode(CData, addTo, addType) \
    _cx_addNode (CData, CXN_CDATA, addTo, addType)
#endif

#if CX_USING_INSTR
/**
 * @func   : cx_addInstrNode
 * @brief  : adds INSTRUCTION type node to tree: <?instr-string?>
 * @called : when populating tree with a INSTRUCTION type node
 * @input  : char *instr - instruction string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addInstrNode(instr, addTo, addType) \
    _cx_addNode (instr, CXN_INSTR, addTo, addType)
#endif

/**
 * @func   : cx_addContentNode
 * @brief  : adds CONTENT type node to tree
 * @called : when populating tree with a CONTENT type node(just data, no tag)
 * @input  : char * content - content string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define cx_addContentNode(content, addTo, addType) \
    _cx_addNode (content, CXN_CONTENT, addTo, addType)

cx_status_t _cx_addNode (const char *tagField,
	cxn_type_t nodeType, const char *addTo, cx_addtype_t addType);

/**
 * @func   : cx_addAttr_CHAR
 * @brief  : adds CHAR type attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_CHAR(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_CHAR, node)

/**
 * @func   : cx_addAttr_STR
 * @brief  : adds STRING type attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_STR(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)attrValue, CXATTR_STR, node)

/**
 * @func   : cx_addAttr_ui8
 * @brief  : adds unsigned 8-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_ui8(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_UI8, node)

/**
 * @func   : cx_addAttr_si8
 * @brief  : adds signed 8-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_si8(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_SI8, node)

/**
 * @func   : cx_addAttr_ui16
 * @brief  : adds unsigned 16-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_ui16(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_UI16, node)

/**
 * @func   : cx_addAttr_si16
 * @brief  : adds signed 16-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_si16(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_SI16, node)

/**
 * @func   : cx_addAttr_ui32
 * @brief  : adds unsigned 32-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_ui32(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_UI32, node)

/**
 * @func   : cx_addAttr_si32
 * @brief  : adds signed 32-bit int attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_si32(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_SI32, node)

/**
 * @func   : cx_addAttr_float
 * @brief  : adds float type attribute to attr list of given node
 * @called : when required to add attr to a node
 * @input  : attrName - name of attr (converted to name string)
 *           cxa_value_u attrValue - union specifying for pre-defined datatype
 *           char *node - tagField of node the attr is added to
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_ATTR/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_FAILURE
 */
#define cx_addAttr_float(attrName, attrValue, node) \
    _cx_addAttrToNode (attrName, (cxa_value_u *)&attrValue, CXATTR_FLOAT, node)

cx_status_t _cx_addAttrToNode (char *attrName, \
	cxa_value_u *value, cxattr_type_t type, char *node);

/**
 * @func   : cx_findNodeWithTag
 * @brief  : use an input tag string to find node that contains that tag
 * @called : when a tagged node is needed to process/update status,etc
 * @input  : char *name - name of the 
 *           cx_node_t *start - parent/root node to start search
 * @output : none
 * @return : NULL - if no match found
 *           !NULL - if node with specified tag is found
 */
cx_node_t *cx_findNodeWithTag (char *name, cx_node_t *start);

void cx_destroyTree (void);

#endif /*__CXML_API_H*/
