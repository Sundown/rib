#ifndef CODEGEN_HEADER
#define CODEGEN_HEADER

#include "../rib.h"

#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Core.h>
#include <llvm-c/DataTypes.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Initialization.h>
#include <llvm-c/Support.h>
#include <llvm-c/Target.h>
#include <llvm-c/Types.h>

typedef struct {
	LLVMBuilderRef builder;
	LLVMModuleRef module;
	LLVMValueRef current_function;
} rib_state;

void llvm_close(rib_state* state);

rib_type read_type(const char* typename);

LLVMValueRef compile(rib_state* state, rib_Noun tree);

LLVMValueRef compile_vector(rib_state* state, rib_Noun tree);
LLVMValueRef compile_function(rib_state* state, rib_Noun tree);
LLVMValueRef compile_global(rib_state* state, rib_Noun tree);

LLVMTypeRef type_build(rib_type type);
rib_type read_type(const char* typename);

LLVMTypeRef type_infer(rib_Noun n);
LLVMValueRef default_val(LLVMTypeRef type);

LLVMValueRef make_puts(LLVMModuleRef mod);
LLVMValueRef make_sqrt(LLVMModuleRef mod);

LLVMValueRef int_literal(int32_t i);
LLVMValueRef real_literal(double i);
LLVMValueRef string_literal(rib_state* state, const char* s);

LLVMValueRef cmpl_add(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs);
LLVMValueRef cmpl_sub(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs);
LLVMValueRef cmpl_mul(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs);
LLVMValueRef cmpl_div(LLVMBuilderRef b, LLVMValueRef lhs, LLVMValueRef rhs);

#endif
