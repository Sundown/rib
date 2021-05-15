#include "codegen.h"

LLVMValueRef make_puts(LLVMModuleRef mod) {
	LLVMValueRef puts_fn = LLVMGetNamedFunction(mod, "puts");
	if (!puts_fn) {
		LLVMTypeRef puts_params[]
		    = {LLVMPointerType(LLVMInt8Type(), 0)};
		puts_fn = LLVMAddFunction(
		    mod,
		    "puts",
		    LLVMFunctionType(LLVMInt32Type(), puts_params, 1, 0));
	}

	return puts_fn;
}

LLVMValueRef make_sqrt(LLVMModuleRef mod) {
	LLVMValueRef sqrt_fn = LLVMGetNamedFunction(mod, "llvm.sqrt.f64");
	if (!sqrt_fn) {
		LLVMTypeRef sqrt_params[] = {LLVMDoubleType()};
		sqrt_fn = LLVMAddFunction(
		    mod,
		    "llvm.sqrt.f64",
		    LLVMFunctionType(LLVMDoubleType(), sqrt_params, 1, 0));
	}

	return sqrt_fn;
}

LLVMValueRef cmpl_add(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
	bool is_real = false;
	if (LLVMTypeOf(lhs) == LLVMDoubleType()) {
		rhs = LLVMBuildCast(b, LLVMSIToFP, rhs, LLVMDoubleType(), "");
		is_real = true;
	} else if (LLVMTypeOf(rhs) == LLVMDoubleType()) {
		lhs = LLVMBuildCast(b, LLVMSIToFP, lhs, LLVMDoubleType(), "");
		is_real = true;
	}

	return is_real ? LLVMBuildFAdd(b, lhs, rhs, "")
		       : LLVMBuildAdd(b, lhs, rhs, "");
}

LLVMValueRef cmpl_sub(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
	bool is_real = false;
	if (LLVMTypeOf(lhs) == LLVMDoubleType()) {
		rhs = LLVMBuildCast(b, LLVMSIToFP, rhs, LLVMDoubleType(), "");
		is_real = true;
	} else if (LLVMTypeOf(rhs) == LLVMDoubleType()) {
		lhs = LLVMBuildCast(b, LLVMSIToFP, lhs, LLVMDoubleType(), "");
		is_real = true;
	}

	return is_real ? LLVMBuildFSub(b, lhs, rhs, "")
		       : LLVMBuildSub(b, lhs, rhs, "");
}

LLVMValueRef cmpl_mul(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
	bool is_real = false;
	if (LLVMTypeOf(lhs) == LLVMDoubleType()) {
		rhs = LLVMBuildCast(b, LLVMSIToFP, rhs, LLVMDoubleType(), "");
		is_real = true;
	} else if (LLVMTypeOf(rhs) == LLVMDoubleType()) {
		lhs = LLVMBuildCast(b, LLVMSIToFP, lhs, LLVMDoubleType(), "");
		is_real = true;
	}

	return is_real ? LLVMBuildFMul(b, lhs, rhs, "")
		       : LLVMBuildMul(b, lhs, rhs, "");
}

LLVMValueRef cmpl_div(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs) {
	if (LLVMTypeOf(lhs) == LLVMInt64Type()) {
		lhs = LLVMBuildCast(b, LLVMSIToFP, lhs, LLVMDoubleType(), "");
	}

	if (LLVMTypeOf(rhs) == LLVMInt64Type()) {
		rhs = LLVMBuildCast(b, LLVMSIToFP, rhs, LLVMDoubleType(), "");
	}

	return LLVMBuildFDiv(b, lhs, rhs, "");
}
