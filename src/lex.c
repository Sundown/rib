#include "rib.h"

rib_Error
rib_lex(const char* rib_String, const char** start, const char** end) {
start:
	rib_String += strspn(rib_String, " \t\r\n");

	if (rib_String[0] == '\0') {
		*start = *end = NULL;
		return MakeErrorCode(ERROR_FILE);
	}

	*start = rib_String;

	if (strchr("(){}[]!:&", rib_String[0]) != NULL) {
		*end = rib_String + 1;
	} else if (rib_String[0] == '"') {
		for (rib_String++; *rib_String != 0; rib_String++) {
			if (*rib_String == '\\') {
				rib_String++;
			} else if (*rib_String == '"') {
				break;
			}
		}

		*end = rib_String + 1;
	} else if (rib_String[0] == ';') {
		rib_String += strcspn(rib_String, "\n");
		goto start;
	} else {
		*end = rib_String + strcspn(rib_String, "(){}[] \t\r\n;");
	}

	return MakeErrorCode(OK);
}
