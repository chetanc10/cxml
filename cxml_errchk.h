
#ifndef __CXML_ERRCHK_H
#define __CXML_ERRCHK_H

#include "cxml_cfg.h"

#if CX_ERR_PRINT_ALLOWED
#define _cx_pr_err(err) printf (err" @ %s +%u\r\n", __func__, __LINE__)
#else
#define _cx_pr_err(...)
#endif

#define cx_lfail(failed, errCode, err) \
	do { \
		if (failed) { \
			_cx_pr_err (err); xStatus = errCode; goto CX_ERR_LBL; \
		} \
	} while (0)

#define cx_func_lfail(function, err) \
	cx_lfail ((CX_SUCCESS != (xStatus = function)), xStatus, err)

#define cx_null_lfail(ptr) \
	cx_lfail ((NULL == ptr), CX_ERR_ALLOC, "Null Buffer: "#ptr)

#endif /*__CXML_ERRCHK_H*/
