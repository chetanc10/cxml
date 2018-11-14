
#ifndef __CXML_ERRCHK_H
#define __CXML_ERRCHK_H

#include "cxml_cfg.h"

/****************************************************************************
* NOTE: All the functions/macros defined with prefix 'cx_' are for cxml usage
* If application needs similar functions/macros, application developer is
* advised to the functions/macros defined with prefix 'cxa_'
****************************************************************************/
#define cx_dbg_print_fn(fmt, ...) \
	SysPrintf ("[%s-%u] "fmt"\r\n", __func__, __LINE__, ##__VA_ARGS__)

#if CX_ENC_DBG_EN
#define cx_enc_dbg(fmt, ...) cx_dbg_print_fn (fmt, ##__VA_ARGS__)
#else
#define cx_enc_dbg(...)
#endif

#if CX_DEC_DBG_EN
#define cx_dec_dbg(fmt, ...) cx_dbg_print_fn (fmt, ##__VA_ARGS__)
#else
#define cx_dec_dbg(...)
#endif

#define cx_rfail(failed, errCode) \
	do { \
		if (failed) { \
			return errCode; \
		} \
	} while (0)

#define cx_func_rfail(function) \
	cx_rfail ((CX_SUCCESS != (xStatus = function)), xStatus)

#define cx_null_rfail(ptr) cx_rfail ((NULL == (ptr)), CX_ERR_NULL_PTR)

#define cx_alloc_rfail(ptr) cx_rfail ((NULL == (ptr)), CX_ERR_ALLOC)

#define cx_lfail(failed, errCode) \
	do { \
		if (failed) { \
			xStatus = errCode; goto CX_ERR_LBL; \
		} \
	} while (0)

#define cx_func_lfail(function) \
	cx_lfail ((CX_SUCCESS != (xStatus = function)), xStatus)

#define cx_null_lfail(ptr) cx_lfail ((NULL == (ptr)), CX_ERR_NULL_PTR)

#define cx_alloc_lfail(ptr) cx_lfail ((NULL == (ptr)), CX_ERR_ALLOC)

/* Application usable calls here are meant to be used in 
 * conjunction with cxml encoder/decode API calls
 * */
#define cxa_perr(fmt, ...) cx_dbg_print_fn (fmt, ##__VA_ARGS__)

#define cxa_rfail(failed, errCode, errStr) \
	do { \
		if (failed) { \
			cxa_perr (errStr); return errCode; \
		} \
	} while (0)

#define cxa_func_rfail(function) \
	cxa_rfail ((CX_SUCCESS != (xStatus = function)), xStatus)

#define cxa_lfail(failed, retVar, errCode, errStr) \
	do { \
		if (failed) { \
			cxa_perr (errStr); retVar = errCode; goto CXA_ERR_LBL; \
		} \
	} while (0)

#define cxa_func_lfail(function, retVar, errCode, errStr) \
	cxa_lfail ((CX_SUCCESS != function), retVar, errCode, errStr)

#endif /*__CXML_ERRCHK_H*/
