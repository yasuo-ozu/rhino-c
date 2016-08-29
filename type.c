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

rh_type *rh_create_type(rh_context *ctx) {
	rh_type *ret = rh_malloc(sizeof(rh_type));
	ret->length = 0;
	ret->is_pointer = 0;
	ret->child = NULL;
	ret->specifier = SP_NULL;
	ret->size = -1;
	ret->sign = 0;
	return ret;
}

void rh_free_type(rh_type *type) {
	if (type->child) rh_free_type(type->child);
	rh_free(type);
}

rh_variable *rh_create_variable(rh_context *ctx, rh_type *type) {
	if (type->size <= 0) {
		E_ERROR(ctx, 0, "Cannot allocate incomplete type");
	}
	rh_variable *ret = rh_malloc(sizeof(rh_variable));
	ret->token = NULL;
	ret->next = NULL;
	ret->memory = type ? rh_malloc(type->size) : NULL;
	ret->depth = -1;
	ret->type = type;
	ret->is_left = 0;
	return ret;
}

void rh_free_variable(rh_variable *var) {
	if (var->memory) rh_free(var->memory);
	if (var->type) rh_free_type(var->type);
	rh_free(var);
}

int rh_compare_type(rh_type *a, rh_type *b) {
	if (a->length == b->length || a->is_pointer && b->is_pointer)
		return rh_compare_type(a->child, b->child);
	if (a->specifier == b->specifier && a->size == b->size &&
			a->sign == b->sign)
		return 1;
	return 0;
}

rh_variable *rh_convert_type(rh_context *ctx, rh_variable *var, rh_type *type, int is_dynamic) {
	rh_variable *ret = rh_malloc(sizeof(rh_variable));
	ret->next = NULL;
	ret->token = NULL;
	ret->depth = -1;
	ret->type = type;
	if (type->is_pointer) {
		if (var->type->is_pointer || var->type->length) {
			if (!is_dynamic && !rh_compare_type(var->type->child, type->child)) {
				E_ERROR(ctx, 0, "Pointer casting error");
			}
		} else if (type->specifier == SP_NUMERIC && !is_dynamic) {
			E_ERROR(ctx, 0, "Pointer casting error");
		}
	} else if (type->length) {
		E_ERROR(ctx, 0, "Array casting error");
		return NULL;
	}
	unsigned char buf[16];
	int i;
	memset(buf, 0, 16);
	memcpy(buf, var->memory, var->type->size);
	// TODO: floating support
	ret->memory = rh_malloc(type->size);
	memcpy(buf, ret->memory, type->size);
	return ret;
}

void rh_assign(rh_context *ctx, rh_variable *to, rh_variable *from) {
	if (!rh_compare_type(from->type, to->type)) {
		from = rh_convert_type(ctx, from, to->type, 0);
	}
	memcpy(to->memory, from->memory, to->type->size);
}

#define RH_TYPE_INT32 ((rh_type *){0,0,0,SP_NUMERIC,4,1})
rh_variable *rh_variable_from_int(rh_context *ctx, int var) {
	rh_variable *ret = rh_create_variable(ctx, RH_TYPE_INT32);
	*((int *)ret->memory) = var;
}

int rh_variable_to_int(rh_context *ctx, rh_variable *var) {
	if (!rh_compare_type(RH_TYPE_INT32, var->type)) {
		var = rh_convert_type(ctx, var, RH_TYPE_INT32, 0);
	}
	return *((int *)var->memory);
}

/* vim: set foldmethod=marker : */
