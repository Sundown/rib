#include "codegen.h"

LLVMValueRef compile_global(rib_state* state, rib_Noun tree) {
	LLVMValueRef var = compile(state, car(cdr(cdr(tree))));
	LLVMValueRef global = LLVMAddGlobal(
	    state->module, LLVMTypeOf(var), car(cdr(tree)).value.symbol);

	LLVMValueRef d = default_val(LLVMTypeOf(var));

	if (d) { /* if it's not a funky type B-) */
		LLVMSetInitializer(global, d);
		LLVMBuildStore(state->builder, var, global);

	} else {
		LLVMSetInitializer(global, var);
	}

	/* could change to returning a load() to be like := operator */
	return NULL;
}

LLVMValueRef compile_vector(rib_state* state, rib_Noun tree) {
	/* TODO: if vector contains function calls it can't be stored in
	 * global */
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
			vec_tmp[i] = compile(state, noun_vec[i]);
		}
	}

	return LLVMConstVector(vec_tmp, vec_len);
}

LLVMValueRef compile_function(rib_state* state, rib_Noun tree) {
	LLVMTypeRef args
	    = type_build(read_type(car(cdr(cdr(tree))).value.symbol));
	args = args != LLVMVoidType() ? args : NULL;

	LLVMTypeRef returns
	    = type_build(read_type(car(cdr(cdr(cdr(tree)))).value.symbol));

	LLVMValueRef fn = LLVMAddFunction(
	    state->module,
	    car(cdr(tree)).value.symbol,
	    LLVMFunctionType(returns, &args, args != NULL, 0));

	state->current_function = fn;

	LLVMPositionBuilderAtEnd(state->builder,
				 LLVMAppendBasicBlock(fn, "entry"));
	rib_Noun body = cdr(car(cdr(cdr(cdr(cdr(tree))))));

	while (!isnil(body)) {
		compile(state, car(body));
		pop(body);
	}

	return fn; /* TODO: add default return */
}
