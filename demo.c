
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
	cx_status_t xStatus;
	char *ptr_xmlBuf = xmlBuf;

	cx_func_lfail (cx_CreateSession (&encCookie, "demo-cxml", ptr_xmlBuf, 0), \
			"XML encoder session creation");
	cx_func_lfail (cx_AddFirstNode (encCookie, "x", CXN_PARENT), "add: x");
	cx_func_lfail (cx_AddAttr_STR (encCookie, "xmlns:xinclude", \
				"http://www.w3.org/2001/XInclude", "x"), "add: xmlns-attr");
	cx_func_lfail (cx_AddCommentNode (encCookie, \
				"Simple test of XML Encoding", \
				"x", CXADD_CHILD), "add: x comment");
	cx_func_lfail (cx_AddParentNode (encCookie, \
				"p", "x", CXADD_CHILD), "add: p");
	cx_func_lfail (cx_AddAttr_STR (encCookie, "xml:base", \
				"../ents/something.xml", "p"), "add: xmlns-attr");
	cx_func_lfail (cx_AddContentNode (encCookie, "simple", \
				"p", CXADD_CHILD), "add: p content");
#if 1
	cx_func_lfail (cx_AddParentNode (encCookie, "q", \
				"p", CXADD_NEXT), "add: q-next-to-p");
#else
	cx_func_lfail (cx_AddParentNode (encCookie, "q", \
				"x", CXADD_CHILD), "add: p");
#endif
	cx_func_lfail (cx_AddContentNode (encCookie, "sample", \
				"q", CXADD_CHILD), "add: q content");

	if (cx_EncPkt (encCookie, NULL)) {
		printf ("Failed encoding..!\n");
		ret = -1;
	} else {
		printf ("Encoded xml packet:\n%s\n", ptr_xmlBuf);
	}

CX_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		printf ("Failed encoding xml string!\n");
	}
	cx_DestroySession (encCookie);

	return (ret = xStatus);
}

int decode_data_in_xml (void)
{
	int ret = 0;

	if (cx_DecPkt (&decCookie, xmlBuf)) {
		printf ("Failed decoding..!\n");
		ret = -1;
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
