
#include <stdio.h>
#include <string.h>
#include "cxml_api.h"
#include "cxml_errchk.h"

void *decCookie;
void *encCookie;
char xmlBuf[1024];

int encode_data_in_xml (void)
{
	int ret = 0;
	cx_status_t xStatus = CX_SUCCESS;
	char *ptr_xmlBuf = xmlBuf;

	cxa_func_lfail (cx_CreateSession (&encCookie, "CXML_DEMO_ENCODE", \
				ptr_xmlBuf, 0), ret, -1, "XML encoder session creation");
	cxa_func_lfail (cx_AddFirstNode (encCookie, "x", CXN_PARENT), ret, -2, \
			"add first node: x");
	cxa_func_lfail (cx_AddAttr_STR (encCookie, "xmlns:xinclude", \
				"http://www.w3.org/2001/XInclude", "x"), ret, -3, \
			"add: xmlns-attr");
	cxa_func_lfail (cx_AddCommentNode (encCookie, \
				"Simple test of XML Encoding", "x", CXADD_CHILD), \
			ret, -4, "add: x comment");
	cxa_func_lfail (cx_AddParentNode (encCookie, "p", "x", CXADD_CHILD), \
			ret, -5, "add: p");
	cxa_func_lfail (cx_AddAttr_STR (encCookie, "xml:base", \
				"../ents/something.xml", "p"), ret, -6, "add: xmlns-attr");
	cxa_func_lfail (cx_AddContentNode (encCookie, "simple", "p", \
				CXADD_CHILD), ret, -7, "add: p content");
#if 1
	cxa_func_lfail (cx_AddParentNode (encCookie, "q", "p", CXADD_NEXT), \
			ret, -8, "add: q-next-to-p");
#else
	cxa_func_lfail (cx_AddParentNode (encCookie, "q", "x", CXADD_CHILD), \
			ret, -8, "add: q as child to x");
#endif
	cxa_func_lfail (cx_AddContentNode (encCookie, "sample", "q", \
				CXADD_CHILD), ret, -9, "add: q content");

	cxa_func_lfail (cx_EncPkt (encCookie, NULL), ret, -111, "Encoding failed");

	printf ("SUCCESS!!!! Encoded xml packet:\n%s\n", ptr_xmlBuf);

CXA_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		printf ("%s\n", cx_strerr (xStatus));
	}
	cx_DestroySession (encCookie);

	return (ret = xStatus);
}

int decode_data_in_xml (void)
{
	int ret = 0;
	cx_status_t xStatus;

	xStatus = cx_DecPkt (&decCookie, xmlBuf, "CXML_DEMO_DECODE");
	if (xStatus) {
		printf ("Failed decoding with reason: %s\n", cx_strerr (xStatus));
		ret = -1;
	} else {
		printf ("Decoding Success!\n");
	}

	cx_DestroySession (decCookie);

	return ret;
}

int main (int argc, char **argv)
{
	int ret = 0;
	char choice;

	if (!argv[1]) {
		printf ("Usage: ./a.out <e|d>\n");
		return -1;
	}

	choice = argv[1][0];
	while (1) {
		switch (choice) {
			case 'e':
				ret = encode_data_in_xml ();
				if (ret) goto END;
				break;
			case 'd':
				ret = decode_data_in_xml ();
				if (ret) goto END;
				break;
			case 'x':
			case 'q':
				printf ("Exiting..\n");
				goto END;
		}
		printf ("e|d: ");
		scanf (" %c", &choice);
	}

END:
	return ret;
}
