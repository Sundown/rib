#include "rib.h"

size_t list_len(rib_Noun xs) {
	rib_Noun* p = &xs;
	size_t ret = 0;
	while (!isnil(*p)) {
		if (p->type != pair_t) { return ret + 1; }

		p = &cdr(*p);
		ret++;
	}

	return ret;
}

rib_Noun reverse_list(rib_Noun list) {
	rib_Noun tail = nil;

	while (!isnil(list)) {
		tail = cons(car(list), tail);
		list = cdr(list);
	}

	return tail;
}

void rib_print_expr(rib_Noun a) {
	char* s = to_string(a, 1);
	printf("%s", s);
}
void rib_print_error(rib_Error e) {
	char* s = error_to_string(e);
	printf("%s", s);
}

void rib_load_file(const char* path) {
	llvm_start(read(path));
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
			append_string(&s, a.value.str->value);
			if (write) append_string(&s, "\"");
			break;
		case number_t:
			sprintf(buf, "%.1f", a.value.number);
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

char* type_to_string(rib_NounType a) {
	switch (a) {
		case nil_t: return "Nil";
		case pair_t: return "Pair";
		case string_t: return "String";
		case noun_t: return "Noun";
		case number_t: return "Float";
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

rib_Noun vector_to_noun(rib_Vector* a, size_t start) {
	rib_Noun r = nil;
	size_t i;
	for (i = start; i < a->size; i++) {
		if (!isnil(a->data[i])) { r = cons(a->data[i], r); }
	}

	return reverse_list(r);
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
	s = a.value.str = calloc(1, sizeof(struct rib_String));
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

		putchar('\n');
		gen(expr);
	}
}
