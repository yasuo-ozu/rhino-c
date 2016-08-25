#include "common.h"

/* returns -1 when non-perfect type */
int rh_get_type_size(rh_type *type) {
	if (type == NULL) return 0;	// Something wrong
	if (type->is_pointer) return 4;
	if (type->specifier == SP_NULL) {
		int r = rh_get_type_size(type->child);
		if (r == -1) return -1;
		else return r * type->length;
	}
	return type->size;
}



/* vim: set foldmethod=marker : */
