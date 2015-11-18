#define _OLEAUT32_

#include <stdio.h>
#include <unknwn.h>

GUID guid;
WORD* wstrGUID[100];
char strGUID[100];
int count, i;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		fprintf(stderr, "SYNTAX: UUIDGEN <number-of-GUIDs-to-generate>\n");
		return 1;
	}
	count = atoi(argv[1]);
	for (i = 0; i < count; i++) {
		CoCreateGuid(&guid);
		StringFromCLSID(&guid, wstrGUID);
		WideCharToMultiByte(CP_ACP, 0, *wstrGUID, -1, strGUID, MAX_PATH, NULL, NULL);
		printf("%s\n", strGUID);
	}
	return 0;
}