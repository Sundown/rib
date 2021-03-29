#include "rib.h"

rib_Noun cons(rib_Noun car_val, rib_Noun cdr_val) {
	rib_Pair* a;
	rib_Noun p;

	a = (rib_Pair*)calloc(1, sizeof(rib_Pair));

	p.type = pair_t;
	p.value.pair = a;

	car(p) = car_val;
	cdr(p) = cdr_val;

	return p;
}

void vector_new(rib_Vector* a) {
	a->capacity = sizeof(a->static_data) / sizeof(a->static_data[0]);
	a->size = 0;
	a->data = a->static_data;
}

void vector_add(rib_Vector* a, rib_Noun item) {
	if (a->size + 1 > a->capacity) {
		a->capacity *= 2;
		if (a->data == a->static_data) {
			a->data
			    = (rib_Noun*)calloc(a->capacity, sizeof(rib_Noun));
			memcpy(a->data,
			       a->static_data,
			       a->size * sizeof(rib_Noun));
		} else {
			a->data = (rib_Noun*)realloc(
			    a->data, a->capacity * sizeof(rib_Noun));
		}
	}

	a->data[a->size] = item;
	a->size++;
}

rib_Noun vector_to_noun(rib_Vector* a, size_t start) {
	rib_Noun r = nil;
	size_t i;
	for (i = start; i < a->size; i++) {
		if (!isnil(a->data[i])) { r = cons(a->data[i], r); }
	}

	return reverse_list(r);
}

void rib_repl() {
	char* input;

	while ((input = readline_fp(_REPL_PROMPT, stdin))) {
		const char* p;
		char* line;
start:
		p = input;
		rib_Noun expr;
		rib_Error err = read_expr(p, &p, &expr);
		if (err._ == MakeErrorCode(ERROR_FILE)._) {
			line = readline_fp("	", stdin);
			if (!line) break;
			input = append_string(&input, "\n");
			input = append_string(&input, line);
			goto start;
		}

		if (!err._) {
			while (1) {

				rib_print_expr(expr);
				puts("");

				err = read_expr(p, &p, &expr);
				if (err._ != MakeErrorCode(OK)._) { break; }
			}
		} else {
			rib_print_error(err);
		}
	}

	putchar('\n');
}

char* type_to_string(rib_NounType a) {
	switch (a) {
		case nil_t: return "Nil";
		case pair_t: return "Pair";
		case string_t: return "String";
		case noun_t: return "Noun";
		case nribber_t: return "Float";
		case bool_t: return "Bool";
		case error_t: return "Error";
		case vector_t: return "Vector";
		default: return "Unknown";
	}
}

char* error_to_string(rib_Error e) {
	char* s = calloc(e.message != NULL ? strlen(e.message) : 0 + 27,
			 sizeof(char));
	e._ != MakeErrorCode(ERROR_USER)._&& e.message
	    ? sprintf(s, "%s\n%s\n", error_string[e._], e.message)
	    : sprintf(s, "%s\n", error_string[e._]);

	return s;
}

rib_Noun intern(const char* s) {
	rib_Noun a;
	a.type = noun_t;
	a.mut = true;
	a.value.symbol = calloc(strlen(s) + 1, sizeof(char));
	strcpy(a.value.symbol, s);

	return a;
}

rib_Noun new_string(char* x) {
	rib_Noun a;
	struct rib_String* s;
	s = a.value.rib_String = calloc(1, sizeof(struct rib_String));
	s->value = x;
	s->mark = 0;

	a.type = string_t;
	a.mut = true;

	return a;
}

void rib_interpret_string(const char* text) {
	rib_Error err = MakeErrorCode(OK);
	const char* p = text;
	rib_Noun expr;
	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		err = read_expr(p, &p, &expr);
		if (err._) { break; }

		rib_print_expr(expr);
		putchar('\n');
	}
}

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
	else if (token[0] == '\'') {
		*result = cons(intern("quote"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == ']') {
		return MakeErrorCode(ERROR_SYNTAX);
	} else if (token[0] == '&') {
		*result = cons(intern("curry"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == '!') {
		*result = cons(intern("not"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == '`') {
		*result = cons(intern("quasiquote"), cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else if (token[0] == ',') {
		*result = cons(token[1] == '@' ? intern("unquote-splicing")
					       : intern("unquote"),
			       cons(nil, nil));
		return read_expr(*end, end, &car(cdr(*result)));
	} else {
		return parse_simple(token, *end, result);
	}
}

char* read(const char* path) {
	FILE* fp = fopen(path, "rb");
	if (!fp) { return NULL; }

	size_t len;
	char* buf;

	fseek(fp, 0, SEEK_END); /* Seek to end */
	len = ftell(fp);	/* record position as length */
	if (len < 0) { return NULL; }
	fseek(fp, 0, SEEK_SET);			    /* seek to start */
	buf = (char*)calloc(len + 1, sizeof(char)); /*alloc based on length*/
	if (!buf) { return NULL; }

	if (fread(buf, 1, len, fp) != len) { return NULL; }
	buf[len] = '\0';

	fclose(fp);
	return buf;
}

char* append_string(char** dst, char* src) {
	size_t len = strlen(*dst) + strlen(src);
	*dst = realloc(*dst, (len + 1) * sizeof(char));
	strcat(*dst, src);
	return *dst;
}

char* rib_new_string() {
	char* s = calloc(1, sizeof(char));
	s[0] = '\0';
	return s;
}

void rib_load_file(const char* path) {
	char* text = read(path);
	if (text) {
		rib_interpret_string(text);

	} else {
		puts("e223 no text!");
	}
}

char* readline_fp(char* prompt, FILE* fp) {
	size_t size = 80;
	char* rib_String;
	int ch;
	size_t len = 0;
	printf("%s", prompt);
	rib_String = calloc(size, sizeof(char));
	if (!rib_String) { return NULL; }

	while ((ch = fgetc(fp)) != EOF && ch != '\n') {
		rib_String[len++] = ch;
		if (len == size) {
			void* p
			    = realloc(rib_String, sizeof(char) * (size *= 2));
			if (!p) { return NULL; }

			rib_String = p;
		}
	}

	if (ch == EOF && len == 0) { return NULL; }

	rib_String[len++] = '\0';

	return realloc(rib_String, sizeof(char) * len);
}

void rib_print_expr(rib_Noun a) {
	char* s = to_string(a, 1);
	printf("%s", s);
}

char* to_string(rib_Noun a, bool write) {
	char *s = rib_new_string(), *s2, buf[80];
	switch (a.type) {
		case nil_t: append_string(&s, "Nil"); break;
		case pair_t: {
			append_string(&s, "(");
			s2 = to_string(car(a), write);
			append_string(&s, s2);

			a = cdr(a);
			while (!isnil(a)) {
				if (a.type == pair_t) {
					append_string(&s, " ");
					s2 = to_string(car(a), write);
					append_string(&s, s2);

					a = cdr(a);
				} else {
					append_string(&s, " . ");
					s2 = to_string(a, write);
					append_string(&s, s2);
					break;
				}
			}

			append_string(&s, ")");

			break;
		}
		case noun_t: append_string(&s, a.value.symbol); break;
		case string_t:
			if (write) append_string(&s, "\"");
			append_string(&s, a.value.rib_String->value);
			if (write) append_string(&s, "\"");
			break;
		case nribber_t:
			sprintf(buf, "%.1f", a.value.nribber);
			append_string(&s, buf);
			break;
		case bool_t:
			append_string(&s, a.value.bool_v ? "True" : "False");
			break;
		case error_t:
			append_string(&s, error_to_string(a.value.error_v));
			break;
		case vector_t: {
			char* l = to_string(vector_to_noun(a.value.vector_v, 0),
					    write);
			append_string(&s, l);
			s[0] = '[';
			s[strlen(s) - 1] = ']';

			break;
		}
		default: append_string(&s, ":Unknown"); break;
	}

	return s;
}

void rib_print_error(rib_Error e) {
	char* s = error_to_string(e);
	printf("%s", s);
}
