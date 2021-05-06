#include "codegen.h"

LLVMModuleRef mod;
LLVMBuilderRef builder;
LLVMValueRef cur_fn;

LLVMValueRef default_val(LLVMTypeRef type) {
	if (type == LLVMInt32Type()) {
		return LLVMConstInt(LLVMInt32Type(), 0, (LLVMBool) false);
	} else if (type == LLVMDoubleType()) {
		return LLVMConstReal(LLVMDoubleType(), 0);
	} else {
		NULL;
	}
}

LLVMValueRef make_puts() {
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

LLVMValueRef make_sqrt() {
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

LLVMValueRef gen(rib_Noun tree) {
	if (tree.type == vector_t) {
		rib_Noun* noun_vec = tree.value.vector_v->data;
		const rib_NounType vec_type = noun_vec[0].type;
		const size_t vec_len = tree.value.vector_v->size;
		LLVMValueRef* vec_tmp = calloc(vec_len, sizeof(LLVMValueRef));
		size_t i;

		for (i = 0; i < vec_len; i++) {
			if (noun_vec[i].type != vec_type) {
				fprintf(
				    stderr,
				    "Error: vector element %s at index %lu is not of type %s\n",
				    to_string(noun_vec[i], 1),
				    i,
				    type_to_string(noun_vec[i].type));

				exit(1);
			} else {
				vec_tmp[i] = gen(noun_vec[i]);
			}
		}

		return LLVMConstVector(vec_tmp, vec_len);
	} else if (tree.type == number_t) {
		return LLVMConstInt(
		    LLVMInt32Type(), (int32_t)tree.value.number, 0);
	} else if (tree.type == string_t) {
		return LLVMBuildGlobalStringPtr(
		    builder, tree.value.str->value, "");
	} else if (car(tree).type == noun_t) {
		if (!strcmp(car(tree).value.symbol, "fn")) {
			rib_Noun fn_takes_type = car(cdr(cdr(tree)));
			rib_Noun fn_gives_type = car(cdr(cdr(cdr(tree))));

			LLVMTypeRef args = type_calculate(fn_takes_type);
			args = args != LLVMVoidType() ? args : NULL;

			LLVMTypeRef returns = type_calculate(fn_gives_type);

			LLVMValueRef fn = LLVMAddFunction(
			    mod,
			    car(cdr(tree)).value.symbol,
			    LLVMFunctionType(returns, &args, args != NULL, 0));

			cur_fn = fn;

			LLVMPositionBuilderAtEnd(
			    builder, LLVMAppendBasicBlock(fn, "entry"));
			rib_Noun body = cdr(car(cdr(cdr(cdr(cdr(tree))))));

			while (!isnil(body)) {
				gen(car(body));
				pop(body);
			}

			return fn;
		} else if (!strcmp(car(tree).value.symbol, "return")) {
			return LLVMBuildRet(builder, gen(car(cdr(tree))));
		} else if (!strcmp(car(tree).value.symbol, "print")) {
			LLVMValueRef args[] = {gen(car(cdr(tree)))};
			LLVMValueRef fn = make_puts();
			return LLVMBuildCall2(builder,
					      LLVMGetReturnType(LLVMTypeOf(fn)),
					      fn,
					      args,
					      1,
					      "");
		} else if (!strcmp(car(tree).value.symbol, "+")) {
			return LLVMBuildAdd(builder,
					    gen(car(cdr(tree))),
					    gen(car(cdr(cdr(tree)))),
					    "");
		} else if (!strcmp(car(tree).value.symbol, "-")) {
			return LLVMBuildSub(builder,
					    gen(car(cdr(tree))),
					    gen(car(cdr(cdr(tree)))),
					    "");
		} else if (!strcmp(car(tree).value.symbol, "*")) {
			return LLVMBuildMul(builder,
					    gen(car(cdr(tree))),
					    gen(car(cdr(cdr(tree)))),
					    "");
		} else if (!strcmp(car(tree).value.symbol, "/")) {
			return LLVMBuildSDiv(builder,
					     gen(car(cdr(tree))),
					     gen(car(cdr(cdr(tree)))),
					     "");
		} else if (!strcmp(car(tree).value.symbol, "var")) {
			LLVMValueRef var = gen(car(cdr(cdr(tree))));
			LLVMValueRef global = LLVMAddGlobal(
			    mod, LLVMTypeOf(var), car(cdr(tree)).value.symbol);

			LLVMValueRef d = default_val(LLVMTypeOf(var));

			if (d) {
				LLVMSetInitializer(global, d);
				LLVMBuildStore(builder, var, global);

			} else {
				LLVMSetInitializer(global, var);
			}

			return NULL;
		} else if (!strcmp(car(tree).value.symbol, "struct")) {
			rib_Noun p = car(cdr(tree));
			size_t elm_count = list_len(p);
			LLVMValueRef* struct_vals
			    = calloc(elm_count, sizeof(LLVMValueRef));

			for (size_t i = 0; !isnil(p); i++, pop(p)) {
				struct_vals[i] = gen(car(p));
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
		} else if (!strcmp(car(tree).value.symbol, "index")) {
			return LLVMBuildExtractElement(builder,
						       gen(car(cdr(tree))),
						       gen(car(cdr(cdr(tree)))),
						       "");
		} else if (!strcmp(car(tree).value.symbol, "real")) {
			return LLVMConstReal(LLVMDoubleType(),
					     car(cdr(tree)).value.number);
		} else if (!strcmp(car(tree).value.symbol, "nat")) {
			return LLVMConstInt(
			    LLVMInt32Type(),
			    (int32_t)car(cdr(tree)).value.number,
			    (LLVMBool) false);
		} else if (!strcmp(car(tree).value.symbol, "sqrt")) {
			LLVMValueRef args[] = {gen(car(cdr(tree)))};
			LLVMValueRef fn = make_sqrt();
			return LLVMBuildCall2(builder,
					      LLVMGetReturnType(LLVMTypeOf(fn)),
					      fn,
					      args,
					      1,
					      "");
		} else {
			LLVMValueRef item
			    = LLVMGetNamedFunction(mod, car(tree).value.symbol);

			if (item) {
				size_t arg_len = list_len(cdr(tree));
				LLVMValueRef* args
				    = calloc(arg_len, sizeof(LLVMValueRef));

				rib_Noun p = cdr(tree);
				for (size_t i = 0; !isnil(p); i++, pop(p)) {
					args[i] = gen(car(p));
				}

				return LLVMBuildCall2(
				    builder,
				    LLVMGetReturnType(LLVMTypeOf(item)),
				    item,
				    args,
				    arg_len,
				    "");
			}
		}
	} else if (tree.type == noun_t) {
		if (!strcmp(tree.value.symbol, "%")) {
			return LLVMGetParam(cur_fn, 0);
		}
		return LLVMBuildLoad(
		    builder, LLVMGetNamedGlobal(mod, tree.value.symbol), "");
	}

	return NULL;
}

void llvm_close(void) {
	LLVMDisposeBuilder(builder);
}

int llvm_start(char* code) {
	mod = LLVMModuleCreateWithName("rib");
	builder = LLVMCreateBuilder();

	rib_interpret_string(code);

	char* error = "failed to write to file";
	LLVMPrintModuleToFile(mod, "bit/out.ll", &error);

	llvm_close();

	return 0;
}
