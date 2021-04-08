#include "rib.h"

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <stdint.h>

LLVMModuleRef mod;
LLVMBuilderRef builder;

LLVMValueRef gen(rib_Noun tree) {
	rib_print_expr(tree), puts("");
	if (tree.type == number_t) {

		return LLVMConstReal(LLVMDoubleType(), tree.value.number);
	} else if (car(tree).type == noun_t) {

		if (!strcmp(car(tree).value.symbol, "return")) {

			return LLVMBuildRet(
			    builder,
			    LLVMConstIntCast(
				gen(car(cdr(tree))), LLVMInt32Type(), 1));
		} else if (!strcmp(car(tree).value.symbol, "+")) {

			rib_print_expr(tree), puts("");
			return LLVMBuildFAdd(builder,
					     gen(car(cdr(tree))),
					     gen(car(cdr(cdr(tree)))),
					     "tmp_add");
		} else if (!strcmp(car(tree).value.symbol, "-")) {
			rib_print_expr(tree), puts("");
			return LLVMBuildFSub(builder,
					     gen(car(cdr(tree))),
					     gen(car(cdr(cdr(tree)))),
					     "tmp_add");
		} else if (!strcmp(car(tree).value.symbol, "*")) {
			rib_print_expr(tree), puts("");
			return LLVMBuildFMul(builder,
					     gen(car(cdr(tree))),
					     gen(car(cdr(cdr(tree)))),
					     "tmp_add");
		} else if (!strcmp(car(tree).value.symbol, "/")) {
			rib_print_expr(tree), puts("");
			return LLVMBuildFDiv(builder,
					     gen(car(cdr(tree))),
					     gen(car(cdr(cdr(tree)))),
					     "tmp_add");
		}
	}
}

void llvm_close(void) {
	LLVMDisposeBuilder(builder);
}

int llvm_start(char* code) {
	mod = LLVMModuleCreateWithName("rib");
	builder = LLVMCreateBuilder();
	LLVMValueRef main_fn = LLVMAddFunction(
	    mod, "main", LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0));

	LLVMPositionBuilderAtEnd(builder,
				 LLVMAppendBasicBlock(main_fn, "entry"));

	rib_interpret_string(code);
	if (LLVMWriteBitcodeToFile(mod, "bit/out.bc") != 0) {
		fprintf(stderr, "Error writing bitcode to file\n");
		exit(1);
	}

	llvm_close();

	return 0;
}
