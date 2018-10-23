
#ifndef __CXML_CFG_H
#define __CXML_CFG_H

/*xml packet size limits*/
#define MAX_TX_XML_PKT_SIZE 2048
#define MAX_RX_XML_PKT_SIZE 3096

/* define as 1 if debug prints are needed in xml_encoder.c */
#define CX_ENC_DBG_EN 1
/* define as 1 if debug prints are needed in xml_decoder.c */
#define CX_DEC_DBG_EN 1

/* if error prints not needed, define as 0 */
#define CX_ERR_PRINT_ALLOWED 1

#define XML_INSTR_STR       "xml version=\"1.0\""

/*define CX_USING_xxx as 1 if the feature xxx should be included*/
#define CX_USING_COMMENTS 1 
#define CX_USING_CDATA    1
#define CX_USING_INSTR    1
#define CX_USING_TAG_ATTR 1

/*can be removed as required.
 *see func _cx_addNode from xml_encoder.c*/
#define STRICT 1

#endif /*__CXML_CFG_H*/
