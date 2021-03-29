#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define _REPL_PROMPT "> "

typedef enum {
	nil_t,
	pair_t,
	noun_t,
	nribber_t,
	vector_t,
	string_t,
	error_t,
	bool_t
} rib_NounType;

typedef enum { OK = 0, ERROR_SYNTAX, ERROR_FILE, ERROR_USER } rib_ErrorCode;

typedef struct {
	rib_ErrorCode _;
	char* message;
} rib_Error;

static const char* error_string[] = {"",
				     "Syntax error",
				     "Symbol unbound",
				     "Parameter error",
				     "Type error",
				     "File error",
				     "",
				     "Cannot mutate constant",
				     "Coercion error"};

typedef struct rib_Noun rib_Noun;
typedef struct rib_Vector rib_Vector;

struct rib_Noun {
	rib_NounType type;
	bool mut;
	union {
		rib_NounType type_v;
		bool bool_v;
		char character;
		rib_Error error_v;
		double nribber;
		struct rib_Pair* pair;
		char* symbol;
		struct rib_String* rib_String;
		rib_Vector* vector_v;
		rib_Error err_v;
	} value;
};

struct rib_Pair {
	struct rib_Noun car, cdr;
};
typedef struct rib_Pair rib_Pair;

struct rib_Vector {
	rib_Noun* data;
	rib_Noun static_data[8];
	size_t capacity, size;
};

struct rib_String {
	char* value;
	char mark;
	struct rib_String* next;
};

static const rib_Noun nil
    = {.type = nil_t, .mut = false, .value = {.type_v = nil_t}};

#define car(p)	 ((p).value.pair->car)
#define cdr(p)	 ((p).value.pair->cdr)
#define isnil(n) ((n).type == nil_t)

#define MakeErrorCode(c) \
	(rib_Error) {    \
		c, NULL  \
	}
#define MakeError(c, m) \
	(rib_Error) {   \
		c, m    \
	}
#define new_vector(v) (rib_Noun){vector_t, true, {.vector_v = v}};
void rib_interpret_string(const char* text);

rib_Noun intern(const char* buf);
rib_Noun new_string(char* x);

rib_Noun reverse_list(rib_Noun list);

void rib_print_expr(rib_Noun a);
void rib_print_error(rib_Error e);

char* rib_new_string();
char* to_string(rib_Noun a, bool write);
char* append_string(char** dst, char* src);

char* readline_fp(char* prompt, FILE* fp);
rib_Error read_expr(const char* input, const char** end, rib_Noun* result);

void rib_load_file(const char* path);
rib_Error rib_lex(const char* rib_String, const char** start, const char** end);
rib_Noun reverse_list(rib_Noun list);
rib_Noun cons(rib_Noun car_val, rib_Noun cdr_val);
void rib_print_expr(rib_Noun a);

void rib_print_error(rib_Error e);
void rib_load_file(const char* path);
char* read(const char* path);
char* append_string(char** dst, char* src);
char* rib_new_string();
char* to_string(rib_Noun a, bool write);
char* type_to_string(rib_NounType a);
char* error_to_string(rib_Error e);

rib_Noun vector_to_noun(rib_Vector* a, size_t start);
void vector_new(rib_Vector* a);
void vector_add(rib_Vector* a, rib_Noun item);

rib_Noun new_string(char* x);
rib_Noun intern(const char* s);
rib_Noun cons(rib_Noun car_val, rib_Noun cdr_val);
