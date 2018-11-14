
#ifndef __CXML_CFG_H
#define __CXML_CFG_H

/*xml packet size limits*/
#define CX_MAX_ENC_STR_SZ 2048
#define CX_MAX_DEC_STR_SZ 3096

/* define as 1 if debug prints are needed in cxml_enc.c */
#define CX_ENC_DBG_EN 0
/* define as 0 if debug prints are needed in cxml_dec.c */
#define CX_DEC_DBG_EN 0
/* define as 0 if debug prints are needed in cxml_common.c */
#define CX_COM_DBG_EN 0

/* if error prints not needed, define as 0 */
#define CX_ERR_PRINT_ALLOWED 1

#define XML_INSTR_STR \
	"xml version=\"1.0\" encoding=\"UTF-8\""

/*define CX_USING_xxx as 1 if the feature xxx should be included*/
#define CX_USING_COMMENTS 1 
#define CX_USING_CDATA    1
#define CX_USING_INSTR    1
#define CX_USING_TAG_ATTR 1

/*define the system relevant printf-or-alike function for logging here*/
/*defaulting to gcc library's printf*/
#define SysPrintf printf

#endif /*__CXML_CFG_H*/
