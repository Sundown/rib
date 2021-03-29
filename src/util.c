#include "rib.h"

rib_Noun reverse_list(rib_Noun list) {
	rib_Noun tail = nil;

	while (!isnil(list)) {
		tail = cons(car(list), tail);
		list = cdr(list);
	}

	return tail;
}
