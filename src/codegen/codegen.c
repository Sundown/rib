#include "codegen.h"

LLVMValueRef compile(rib_state* state, rib_Noun tree) {
	if (tree.type == vector_t) {
		return compile_vector(state, tree);
	} else if (tree.type == number_t) {
		return int_literal((int32_t)tree.value.number);
	} else if (tree.type == string_t) {
		return string_literal(state, tree.value.str->value);
	} else if (car(tree).type == noun_t) {
		if (!strcmp(car(tree).value.symbol, "define")) {
			return compile_function(state, tree);
		} else if (!strcmp(car(tree).value.symbol, "ret")) {
			return LLVMBuildRet(state->builder,
					    compile(state, car(cdr(tree))));
		} else if (!strcmp(car(tree).value.symbol, "print")) {
			LLVMValueRef args[] = {compile(state, car(cdr(tree)))};
			LLVMValueRef fn = make_puts(state->module);
			return LLVMBuildCall2(state->builder,
					      LLVMGetReturnType(LLVMTypeOf(fn)),
					      fn,
					      args,
					      1,
					      "");
		} else if (!strcmp(car(tree).value.symbol, "+")) {
			return cmpl_add(state->builder,
					compile(state, car(cdr(tree))),
					compile(state, car(cdr(cdr(tree)))));
		} else if (!strcmp(car(tree).value.symbol, "-")) {
			return cmpl_sub(state->builder,
					compile(state, car(cdr(tree))),
					compile(state, car(cdr(cdr(tree)))));
		} else if (!strcmp(car(tree).value.symbol, "*")) {
			return cmpl_mul(state->builder,
					compile(state, car(cdr(tree))),
					compile(state, car(cdr(cdr(tree)))));
		} else if (!strcmp(car(tree).value.symbol, "/")) {
			return cmpl_div(state->builder,
					compile(state, car(cdr(tree))),
					compile(state, car(cdr(cdr(tree)))));
		} else if (!strcmp(car(tree).value.symbol, "global")) {
			return compile_global(state, tree);
		} else if (!strcmp(car(tree).value.symbol, "struct")) {
			rib_Noun p = car(cdr(tree));
			size_t elm_count = list_len(p);
			LLVMValueRef* struct_vals
			    = calloc(elm_count, sizeof(LLVMValueRef));

			for (size_t i = 0; !isnil(p); i++, pop(p)) {
				struct_vals[i] = compile(state, car(p));
			}

			LLVMTypeRef* struct_types
			    = calloc(elm_count, sizeof(LLVMTypeRef));

			for (size_t i = 0; i < elm_count; i++) {
				struct_types[i] = LLVMTypeOf(struct_vals[i]);
			}

			return LLVMConstNamedStruct(
			    LLVMStructType(
				struct_types, elm_count, (LLVMBool) false),
			    struct_vals,
			    elm_count);
		} else if (!strcmp(car(tree).value.symbol, "cast")) {
			LLVMValueRef v = compile(state, car(cdr(tree)));
			LLVMTypeRef t = type_build(
			    read_type(car(cdr(cdr(tree))).value.symbol));
			if (t == LLVMTypeOf(v)) { return v; }

			if (t == LLVMInt64Type()) {
				if (LLVMTypeOf(v) == LLVMDoubleType()) {
					return LLVMBuildCast(state->builder,
							     LLVMFPToSI,
							     v,
							     t,
							     "");
				} else {
					printf("Can't cast to int\n");
					exit(1);
				}
			} else if (t == LLVMDoubleType()) {
				if (LLVMTypeOf(v) == LLVMInt64Type()) {
					return LLVMBuildCast(state->builder,
							     LLVMSIToFP,
							     v,
							     t,
							     "");

				} else {
					printf("Can't cast to real\n");
					exit(1);
				}
			} else {
				printf("Can't cast\n");
				exit(1);
			}
		} else if (!strcmp(car(tree).value.symbol, "index")) {
			return LLVMBuildExtractElement(
			    state->builder,
			    compile(state, car(cdr(tree))),
			    compile(state, car(cdr(cdr(tree)))),
			    "");
		} else if (!strcmp(car(tree).value.symbol, "real")) {
			return real_literal(car(cdr(tree)).value.number);
		} else if (!strcmp(car(tree).value.symbol, "nat")) {
			return int_literal(
			    (int32_t)car(cdr(tree)).value.number);
		} else if (!strcmp(car(tree).value.symbol, "sqrt")) {
			LLVMValueRef args[] = {compile(state, car(cdr(tree)))};
			LLVMValueRef fn = make_sqrt(state->module);
			return LLVMBuildCall2(state->builder,
					      LLVMGetReturnType(LLVMTypeOf(fn)),
					      fn,
					      args,
					      1,
					      "");
		} else {
			LLVMValueRef item = LLVMGetNamedFunction(
			    state->module, car(tree).value.symbol);

			if (item) {
				size_t arg_len = list_len(cdr(tree));
				LLVMValueRef* args
				    = calloc(arg_len, sizeof(LLVMValueRef));

				rib_Noun p = cdr(tree);
				for (size_t i = 0; !isnil(p); i++, pop(p)) {
					args[i] = compile(state, car(p));
				}

				return LLVMBuildCall2(
				    state->builder,
				    LLVMGetReturnType(LLVMTypeOf(item)),
				    item,
				    args,
				    arg_len,
				    "");
			}
		}
	} else if (tree.type == noun_t) {
		if (!strcmp(tree.value.symbol, "%")) {
			return LLVMGetParam(state->current_function, 0);
		}

		return LLVMBuildLoad(
		    state->builder,
		    LLVMGetNamedGlobal(state->module, tree.value.symbol),
		    "");
	}

	return NULL;
}

void llvm_start(char* code) {
	rib_state state
	    = {LLVMCreateBuilder(), LLVMModuleCreateWithName("rib"), NULL};

	rib_Vector* v = rib_interpret_string(code);
	size_t i;

	for (i = 0; i < v->size; i++) { compile(&state, v->data[i]); }

	char* error = "failed to write to file";
	LLVMPrintModuleToFile(state.module, "bit/out.ll", &error);

	llvm_close(&state);
}

void llvm_close(rib_state* state) {
	LLVMDisposeBuilder(state->builder);
	LLVMDisposeModule(state->module);
}
