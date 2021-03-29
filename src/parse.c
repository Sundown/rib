#include "rib.h"

rib_Error parse_simple(const char* start, const char* end, rib_Noun* result) {
	char *p, *buf, *pt;
	size_t length = end - start - 2;
	rib_Error err;
	rib_Noun a1, a2;
	long len, i;
	const char* ps;

	double val = strtod(start, &p);
	if (p == end) {
		result->type = nribber_t;
		result->value.nribber = val;
		return MakeErrorCode(OK);
	} else if (start[0] == '"') {
		result->type = string_t;
		buf = (char*)calloc(length + 1, sizeof(char));
		ps = start + 1;
		pt = buf;

		while (ps < end - 1) {
			if (*ps == '\\') {
				char c_next = *(ps + 1);

				switch (c_next) {
					case 'r': *pt = '\r'; break;
					case 'n': *pt = '\n'; break;
					case 't': *pt = '\t'; break;
					default: *pt = c_next;
				}

				ps++;
			} else {
				*pt = *ps;
			}

			ps++;
			pt++;
		}
		*pt = 0;
		buf = realloc(buf, pt - buf + 1);
		*result = new_string(buf);
		return MakeErrorCode(OK);
	}

	buf = calloc(end - start + 1, sizeof(char));
	memcpy(buf, start, end - start);
	buf[end - start] = 0;

	if (strcmp(buf, "nil") == 0) {
		*result = nil;
	} else if (strcmp(buf, ".") == 0) {
		*result = intern(buf);
	} else {
		len = end - start;
		for (i = len - 1; i >= 0; i--) {
			if (buf[i] == ':' && buf[i + 1] == ':') {
				if (i == 0 || i == len - 1) {

					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf, buf + i, &a1);
				if (err._) {
					return MakeErrorCode(ERROR_SYNTAX);
				}

				err = parse_simple(buf + i + 2, buf + len, &a2);
				if (err._) {
					return MakeErrorCode(ERROR_SYNTAX);
				}

				*result = cons(
				    a1,
				    cons(cons(intern("quote"), cons(a2, nil)),
					 nil));
				return MakeErrorCode(OK);
			}
		}

		*result = intern(buf);
	}

	return MakeErrorCode(OK);
}

rib_Error read_list(const char* start, const char** end, rib_Noun* result) {
	rib_Noun p;

	*end = start;
	p = *result = nil;

	for (;;) {
		const char* token;
		rib_Noun item = nil;
		rib_Error err;

		err = rib_lex(*end, &token, end);
		if (err._) { return err; }

		if (token[0] == ')') { return MakeErrorCode(OK); }

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		if (isnil(p)) {
			*result = cons(item, nil);
			p = *result;
		} else {
			cdr(p) = cons(item, nil);
			p = cdr(p);
		}
	}
}

rib_Error read_prefix(const char* start, const char** end, rib_Noun* result) {
	rib_Noun p = *result = nil;
	*end = start;

	while (1) {
		const char* token;
		rib_Noun item;
		rib_Error err = rib_lex(*end, &token, end);

		if (err._) { return err; }

		if (token[0] == ']') {
			*result = cons(intern("list"), reverse_list(p));
			return MakeErrorCode(OK);
		}

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		p = cons(item, p);
	}
}

rib_Error read_block(const char* start, const char** end, rib_Noun* result) {
	rib_Noun p = *result = nil;
	*end = start;

	p = nil;

	while (1) {
		const char* token;
		rib_Noun item;
		rib_Error err = rib_lex(*end, &token, end);

		if (err._) { return err; }

		if (token[0] == '}') {
			*result = cons(intern("do"), reverse_list(p));
			return MakeErrorCode(OK);
		}

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		p = cons(item, p);
	}
}

rib_Error read_vector(const char* start, const char** end, rib_Noun* result) {
	*result = nil;
	rib_Vector* v = calloc(1, sizeof(rib_Vector));
	vector_new(v);
	*end = start;

	while (1) {
		const char* token;
		rib_Noun item;
		rib_Error err = rib_lex(*end, &token, end);

		if (err._) { return err; }

		if (token[0] == ']') {
			*result = new_vector(v);
			return MakeErrorCode(OK);
		}

		err = read_expr(token, end, &item);

		if (err._) { return err; }

		vector_add(v, item);
	}
}

rib_Error read_expr(const char* input, const char** end, rib_Noun* result) {
	char* token;
	rib_Error err;

	err = rib_lex(input, (const char**)&token, end);
	if (err._) { return err; }

	if (token[0] == '(') {
		return read_list(*end, end, result);
	} else if (token[0] == ')') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '[') {
		return read_vector(*end, end, result);
	} else if (token[0] == ']') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '{') {
		return read_block(*end, end, result);
	} else if (token[0] == '}') {
		return MakeErrorCode(ERROR_SYNTAX);
	} /* else if (token[0] == '[') {
		 rib_Noun n0, n1;
		 rib_Error e0 = read_prefix(*end, end, &n0);
		 if (e0._) { return e0; }
		 n0 = car(cdr(n0));
		 e0 = eval_expr(n0, env, &n1);
		 if (e0._) { return e0; }
		 switch (n1.type) {
			 case type_t: {
				 *result = cons(intern("cast"),
						cons(nil, cons(n1, nil)));
				 return read_expr(*end, end,
	 &car(cdr(*result)));
			 }
			 default:
				 return MakeError(
				     ERROR_ARGS,
				     "prefix: was not passed a valid prefix");
		 }

	 } else if (token[0] == ']') {
		 return MakeErrorCode(ERROR_SYNTAX);
	 }*/

	else if (token[0] == ']') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '&') {
		*result = cons(intern("curry"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == '!') {
		*result = cons(intern("not"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else {
		return parse_simple(token, *end, result);
	}
}
