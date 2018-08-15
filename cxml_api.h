
#ifndef __CXML_API_H
#define __CXML_API_H

#include <stdint.h>
#include <errno.h>

/* This file gives the interface to use cxml encoder/decoder
 * Just don't worry about symbol with '_' as first character */

/*xml packet size limits*/
#define MAX_TX_XML_PKT_SIZE 2048
#define MAX_RX_XML_PKT_SIZE 3096

#pragma pack(1)

typedef union attrValue_union {
    uint32_t  xVal; /*to refer all variables here in generic*/
    uint32_t  n_u32;
    int32_t   n_i32;
    uint16_t  n_u16;
    int16_t   n_i16;
    int8_t    n_i8;
    uint8_t   n_u8;
    float     f;
    char      *str;
    char      ch;
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
    CXA_CHAR,
    CXA_STR,
    CXA_UI8,
    CXA_SI8,
    CXA_UI16,
    CXA_SI16,
    CXA_UI32,
    CXA_SI32,
    CXA_FLOAT,
    CXA_MAX,
} cxa_type_t;

typedef enum {
    CX_ADD_MINTYPE,
    CX_ADD_AS_CHILD,
    CX_ADD_AS_NEXT,
    CX_ADD_AS_FIRST,
    CX_ADD_MAXTYPE,
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
    cxn_addr_t          *attrList;
#endif
    uint8_t             numOfChildren;
    struct cx_node_s    *children;
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
cx_status_t encode_xml_pkt(char **xmlData);

/**
 * @func   : decode_xml_pkt
 * @brief  : build an xml tree from an existing xml-string
 * @called : when a peer sends xml content packet and we need to
 *           validate and parse the same properly to build xml-tree
 * @input  : char *str - existing xml string
 * @output : cx_node_t **xmlNode - pointer to root-node holder
 * @return : CX_SUCCESS/CX_FAILURE/CX_ERR_ALLOC
 */
cx_status_t decode_xml_pkt(char *str, cx_node_t **xmlNode);

/**
 * @func   : addFirstNode
 * @brief  : adds first(root?) node to tree
 * @called : when populating tree for the first time
 * @input  : char *name - tagField
 *           cxn_type_t nodeType - type of node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define addFirstNode(name, nodeType) \
    addNodeToTree(name, nodeType, NULL, CX_ADD_AS_FIRST)

/**
 * @func   : addParentNode
 * @brief  : adds PARENT type node to tree
 * @called : when populating tree with a PARENT type node
 * @input  : char *name - tagField
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define addParentNode(name, addTo, addType) \
    addNodeToTree(name, CXN_PARENT, addTo, addType)

/**
 * @func   : addSingleNode
 * @brief  : adds SINGLE type node to tree
 * @called : when populating tree with a SINGLE type node (tag is '<node/>')
 * @input  : char *name - tagField
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define addSingleNode(name, addTo, addType) \
    addNodeToTree(name, CXN_SINGLE, addTo, addType)

#if CX_USING_COMMENTS
/**
 * @func   : addCommentNode
 * @brief  : adds COMMENT type node to tree
 * @called : when populating tree with a COMMENT type node: <!-- comment -->
 * @input  : char *comment - comment string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define addCommentNode(comment, addTo, addType) \
    addNodeToTree(comment, CXN_COMMENT, addTo, addType)
#endif

#if CX_USING_CDATA
/**
 * @func   : addCDataNode
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
#define addCDataNode(CData, addTo, addType) \
    addNodeToTree(CData, CXN_CDATA, addTo, addType)
#endif

#if CX_USING_INSTR
/**
 * @func   : addInstrNode
 * @brief  : adds INSTRUCTION type node to tree: <?instr-string?>
 * @called : when populating tree with a INSTRUCTION type node
 * @input  : char *instr - instruction string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define addInstrNode(instr, addTo, addType) \
    addNodeToTree(instr, CXN_INSTR, addTo, addType)
#endif

/**
 * @func   : addContentNode
 * @brief  : adds CONTENT type node to tree
 * @called : when populating tree with a CONTENT type node(just data, no tag)
 * @input  : char * content - content string
 *           char *addTo - name of node to which current node is added
 *           cx_addtype_t addType - to add as child/next/first node
 * @output : none
 * @return : CX_SUCCESS/CX_ERR_BAD_NODETYPE/CX_ERR_NULL_MEM/
 *           CX_ERR_ALLOC/CX_ERR_BAD_NODE/CX_FAILURE
 */
#define addContentNode(content, addTo, addType) \
    addNodeToTree(content, CXN_CONTENT, addTo, addType)

cx_status_t addNodeToTree(const char *tagField,
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
    _cx_addAttrToNode(attrName, &attrValue, CXA_CHAR, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_STR, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_UI8, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_SI8, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_UI16, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_SI16, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_UI32, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_SI32, node)

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
    _cx_addAttrToNode(attrName, &attrValue, CXA_FLOAT, node)

cx_status_t _cx_addAttrToNode(char *attrName, \
	cxa_value_u *value, cxa_type_t type, char *node);

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
cx_node_t *cx_findNodeWithTag(char *name, cx_node_t *start);

void cx_destroyTree(void);

cx_status_t encode_xml_pkt(char **xmlData);
cx_status_t decode_xml_pkt(char *str, cx_node_t **xmlNode);

#endif /*__CXML_API_H*/
