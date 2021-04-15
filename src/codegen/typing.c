#include "codegen.h"

LLVMTypeRef type_calculate(rib_Noun n) {
	switch (n.value.symbol[0]) {
		case 'i': {
			if (!strcmp(n.value.symbol, "i8")) {
				return LLVMInt8Type();
			} else if (!strcmp(n.value.symbol, "i16")) {
				return LLVMInt16Type();
			} else if (!strcmp(n.value.symbol, "i32")) {
				return LLVMInt32Type();
			} else if (!strcmp(n.value.symbol, "i64")) {
				return LLVMInt64Type();
			} else if (!strcmp(n.value.symbol, "i128")) {
				return LLVMInt128Type();
			}
		}
		case 's': {
			return LLVMPointerType(LLVMInt8Type(), 0);
		}
		default: {
			break;
		}
	}

	// if (!strcmp(sym, "void")) { return LLVMVoidType(); }
	return LLVMVoidType();
}

LLVMTypeRef type_infer(rib_Noun n) {
	switch (n.type) {
		case number_t: {
			/* TODO: handle floats properly once we have ints in the
			 * parser */
			return LLVMInt32Type();
		}
		case string_t: {
			/* Not certain this is the correct type, seems to be
			 * what puts() takes */
			return LLVMPointerType(LLVMInt8Type(), 0);
		}
		default: {

			fputs("Got here A1", stderr);
			return NULL;
		}
	}
}
