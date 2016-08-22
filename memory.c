#include "common.h"

void *rh_malloc(size_t size) {
	return malloc(size);
}

void rh_free(void *p) {
	free(p);
}

void *rh_realloc(void *p, size_t size) {
	return realloc(p, size);
}

// void rh_memman_init(rh_memman *man) {
// 	int i;
// 	man->tanks = 0;
// 	for (i = 0; i < MEMORY_TANK_MAX; i++) man->tank[i].type = MTYPE_NULL;
// 	man->data = man->data_top = MEMORY_DATA_START;
// 	man->heap = man->pbreak = MEMORY_HEAP_START;
// 	man->text = man->text_top = MEMORY_TEXT_START;
// 	man->stack = MEMORY_SIZ;
// }
// 
// void rh_memman_free(rh_memman *man) {
// 	int i;
// 	for (i = 0; i < man->tanks; i++) {
// 		rh_free(man->tank[i].ptr);
// 	}
// }
// 
// #define MAX(a,b)	((a)>(b)?(a):(b))
// #define MIN(a,b)	((a)<(b)?(a):(b))
// 
// rh_size_t rh_malloc_type(rh_memman *man, rh_size_t size, rh_mem_type type) {
// 	rh_size_t allignment = size == 4 ? 4 : (size == 2 ? 2 : 1);
// 	rh_size_t begin, end, min, max, new_begin, new_end;
// 	int i;
// 	size_t minsiz, newsiz;
// 	if (type == MTYPE_STACK) {
// 		begin = (man->stack - size) / allignment * allignment;
// 		end = begin + size; man->stack = begin;
// 		min = man->pbreak; max = MEMORY_SIZ;
// 		minsiz = MEMORY_MIN_STACK_SIZE;
// 	} else if (type == MTYPE_DATA) {
// 		begin = (man->data_top + allignment - 1) / allignment * allignment;
// 		end = begin + size; man->data_top = end;
// 		min = man->data; max = man->heap;
// 		minsiz = MEMORY_MIN_DATA_SIZE;
// 	} else if (type == MTYPE_TEXT) {
// 		begin = (man->text_top + allignment - 1) / allignment * allignment;
// 		end = begin + size; man->text_top = end;
// 		min = man->text; max = man->data;
// 		minsiz = MEMORY_MIN_TEXT_SIZE;
// 	} else {
// 		fprintf(stderr, "internal: Malloc Invalid Type\n"); exit(1);
// 	}
// 
// 	/* memory range check */
// 	if (begin < min || end > max) {
// 		fprintf(stderr, "Memory allocation error\n"); exit(1);
// 	}
// 
// 	/* search memory and allocate */
// 	for (i = 0; i < man->tanks; i++) {
// 		if (begin < man->tank[i].end && end > man->tank[i].begin) {
// 			if (man->tank[i].type != type) {
// 				fprintf(stderr, "internal: Memory type error\n"); exit(1);
// 			}
// 			if (man->tank[i].begin <= begin && end <= man->tank[i].end) return begin;
// 			else {
// 				new_begin = MAX(man->tank[i].begin, begin);
// 				new_end = MIN(man->tank[i].end, end);
// 				man->tank[i].ptr = rh_realloc(man->tank[i].ptr, new_end - new_begin);
// 				man->tank[i].end = new_end; man->tank[i].begin = new_begin;
// 			}
// 		}
// 	}
// 
// 	new_end = (type != MTYPE_STACK && end - begin < minsiz) ? begin + minsiz : end;
// 	new_begin = (type == MTYPE_STACK && end - begin < minsiz) ? end -  minsiz : begin;
// 	man->tank[man->tanks].ptr = rh_malloc(new_end - new_begin);
// 	man->tank[man->tanks].begin = new_begin;
// 	man->tank[man->tanks].end = new_end;
// 	man->tank[man->tanks].size = new_end - new_begin;
// 	man->tanks++;
// 	return begin;
// }
// 
// void rh_dump_memory_usage(FILE *fp, rh_memman *man){
// 	int i;
// 	rh_size_t stack = 0, data = 0, text = 0;
// 	for (i = 0; i < man->tanks; i++) {
// 		if (man->tank[i].type == MTYPE_STACK) stack += man->tank[i].size;
// 		if (man->tank[i].type == MTYPE_DATA) data += man->tank[i].size;
// 		if (man->tank[i].type == MTYPE_TEXT) text += man->tank[i].size;
// 	}
// 	fprintf(fp, "STACK: ALLOC=%d USE=%d\n", stack, MEMORY_SIZ - man->stack);
// 	fprintf(fp, "DATA: ALLOC=%d USE=%d\n", stack, man->data_top - man->data);
// 	fprintf(fp, "TEXT: ALLOC=%d USE=%d\n", stack, man->text_top - man->text);
// 	fprintf(fp, "HEAP: ???\n");
// }
// 
// /* vim: set foldmethod=marker : */
