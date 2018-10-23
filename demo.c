
#include <stdio.h>
#include <string.h>
#include "cxml_api.h"
#include "cxml_errchk.h"

char xmlPkt[1024] = "<?xml version=\"1.0\"?><x xmlns:xinclude=\"http://www.w3.org/2001/XInclude\"><!-- Simple test of including a set of nodes from an XML document --><p xml:base=\"../ents/something.xml\">simple</p></x>";
cx_node_t *root;

int encode_data_in_xml (void)
{
	int ret = 0;
	cx_status_t xStatus;
	char encBuf[1024];
	char *ptr_encBuf;

	cx_func_lassert (cx_addFirstNode ("x", CXN_PARENT), "add: x");
	cx_func_lassert (cx_addAttr_STR ("xmlns:xinclude", "http://www.w3.org/2001/XInclude", "x"), "add: xmlns-attr");
	cx_func_lassert (cx_addCommentNode ("Simple test of including a set of nodes from an XML document", "x", CXADD_CHILD), "add: x comment");
	cx_func_lassert (cx_addParentNode ("p", "x", CXADD_CHILD), "add: p");
	cx_func_lassert (cx_addAttr_STR ("xml:base", "../ents/something.xml", "p"), "add: xmlns-attr");
	cx_func_lassert (cx_addContentNode ("simple", "p", CXADD_CHILD), "add: p content");

	if (encode_xml_pkt (&ptr_encBuf)) {
		printf ("Failed encoding..!\n");
		ret = -1;
	} else {
		printf ("Encoded xml packet:\n%s\n", ptr_encBuf);
	}

CX_ERR_LBL:
	if (xStatus != CX_SUCCESS) {
		printf ("Failed encoding xml string!\n");
	}
	return ret;
}

int decode_data_in_xml (void)
{
	int ret = 0;

	if (decode_xml_pkt (xmlPkt, &root)) {
		printf ("Failed decoding..!\n");
		ret = -1;
	}

	return ret;
}

int main (int argc, char **argv)
{
	int ret = 0;
	char choice;
	cx_status_t cxst = CX_SUCCESS;

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