#include "codegen.h"

rib_type read_type(const char* typename) {
	if (!strcmp(typename, "void")) {
		return type_void;
	} else if (!strcmp(typename, "int")) {
		return type_int;
	} else if (!strcmp(typename, "nat")) {
		return type_nat;
	} else if (!strcmp(typename, "real")) {
		return type_real;
	} else if (!strcmp(typename, "bool")) {
		return type_bool;
	} else if (!strcmp(typename, "char")) {
		return type_char;
	} else if (!strcmp(typename, "word")) {
		return type_word;
	} else if (!strcmp(typename, "buffer")) {
		return type_bitbuffer;
	} else {
		return type_void;
	}
}

LLVMTypeRef type_build(rib_type type) {
	switch (type) {
		case type_void: return LLVMVoidType();
		case type_int:
		case type_nat: return LLVMInt64Type();
		case type_real: return LLVMDoubleType();
		case type_char: return LLVMInt8Type();
		case type_bool: return LLVMInt1Type();
		/* TODO: check this works */
		case type_word: return LLVMIntType(sizeof(void*) * 8);
		/* TODO: add bitbuffers */
		default: return LLVMVoidType();
	}
}

LLVMValueRef int_literal(int32_t i) {
	return LLVMConstInt(LLVMInt64Type(), i, (LLVMBool) false);
}

LLVMValueRef real_literal(double i) {
	return LLVMConstReal(LLVMDoubleType(), i);
}

LLVMValueRef string_literal(rib_state* state, const char* s) {
	return LLVMBuildGlobalStringPtr(state->builder, s, "");
}

LLVMValueRef default_val(LLVMTypeRef type) {
	if (type == LLVMInt64Type()) {
		return int_literal(0);
	} else if (type == LLVMDoubleType()) {
		return real_literal(0);
	} else {
		return NULL;
	}
}
