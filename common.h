#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <setjmp.h>
#include <stddef.h>
#include <sys/types.h>

/****************************************/
/*************** file.c *****************/
/****************************************/
#define MAX_FNAME	1024
#define MAX_FILE_DEPTH	40
#define DEFAULT_FILE_BUF_SIZE	(4*1024)

typedef struct rh_file {
	// FILE *fp;
	// int line, ch, byte;
	char name[MAX_FNAME];
	struct rh_file *parent;
	char *buf, *buf_end;
	/* buffer for rh_ungetc() */
	// int unget_buf_top;
	// int unget_buf[MAX_UNGET_BUF];

	/* file-spicified flags */
	int dump_token;
} rh_file;

/****************************************/
/************** token.c *****************/
/****************************************/
typedef struct rh_token {
	enum {
		TKN_IDENT = 1,		/* begins with _A-Za-z, followed with _A-Za-z0-9 */
		TKN_NUMERIC = 2,	/* begins with 0-9, followed with _A-Za-z0-9 */
		TKN_SYMBOL = 3,		/* Normally 1 char. Multipul symbol is specially defined in token.c */
		TKN_CHAR = 4,		/* begins with ', ends with ' */
		TKN_STRING = 5,		/* begins with ", ends with ". following string tokens are connected as one. */
		TKN_NULL = 0		/* implies end of file */
	} type;
	struct rh_token *next;
	rh_file *file;
	char *file_begin, *file_end;	/* Pointer on rh_file->buf */
	// struct {
	// 	enum {
	// 		TYPE_INT, TYPE_DOUBLE
	// 	} type;
	// 	long long intval;
	// 	long double dblval;
	// } literal;
	rh_declerator *declerator;
	// int line1, ch1, byte1, line2, ch2, byte2;
	// char text[];	/* incomplete type */
} rh_token;

enum {/*{{{*/
	CP_CAPITAL		= 1,	/* A-Z */
	CP_LITERAL		= 2,	/* a-z */
	CP_8DIGIT		= 4,	/* 01234567*/
	CP_10DIGIT		= 8,	/* 0123456789 */
	CP_16DIGIT		= 16,	/* 0123456789ABCDEFabcdef */
	CP_SPACE		= 32,	/* isspace() */
	CP_IDENT_FIRST	= 64,	/* _A-Za-z */
	CP_IDENT		= 128,	/* _A-Za-z0-9 */
	CP_SYMBOL		= 256	/* !"#$%&'()*+,-./:;<=>?p[\]^`{|}~ */
} ctbl[0xFF];/*}}}*/

/****************************************/
/*************** memory.c ***************/
/****************************************/
 
/****************************************/
/*************** type.c ***************/
/****************************************/

/* It may refered from multipul places. Do not release! */
typedef struct rh_type {
	int length;		/* Array length. 0 means non-array */
	int is_pointer;
	struct rh_type *child;	/* Non-null when length>0 || is_pointer */

	/* Set when length==0&&!is_pointer */
	enum {
		SP_NULL = 0, SP_NUMERIC, SP_FLOATING
	} specifier;	
	int size, sign;
} rh_type;

int rh_get_type_size(rh_type *type);

/****************************************/
/**************** execute.c *************/
/****************************************/

#ifdef _MSC_BUILD
typedef int ssize_t;
#endif
typedef struct rh_declarator {
	rh_token *token;	/* When token null, *memory points to the area alloced alone */
	struct rh_declarator *next;
	unsigned char *memory;
	int depth;
	rh_type *type;
} rh_declarator;

/****************************************/
/**************** error.c ***************/
/****************************************/
#define ERROR_MAX_LENGTH	1024
#define ERROR_MAX_COUNT		128
#define ERROR_THRESHOLD		5
#define TERM_MAX_WIDTH		80

typedef enum {
	ETYPE_NOTICE = 1, ETYPE_WARNING = 2, ETYPE_ERROR = 3,
	ETYPE_FATAL = 4,	/* error which stops compile immediately */
	ETYPE_TRAP = 5,		/* error which occurs in and stops executing */
	ETYPE_INTERNAL = 6	/* mainly thrown by rh_assert() */
} rh_error_type;

typedef struct {
	rh_error_type type;
	char message[ERROR_MAX_LENGTH + 1];
	int line1, ch1, line2, ch2;
	// file 
} rh_error_context_item;

typedef struct {
	rh_error_context_item error[ERROR_MAX_COUNT];
	int errors;
	jmp_buf jmpbuf;

} rh_error_context;

/****************************************/
/**************** main.c ****************/
/****************************************/

#define MEMORY_SIZE	((size_t)0xFFF)
typedef struct {
	// rh_memman memman;
	rh_file *file;
	// rh_compile_context compile;
	rh_token *token;
	rh_error_context error;
	// rh_type *type_top;
	char *ch;
	rh_declarator *declarator;
	unsigned char *memory, *sp, *hp;
	int depth;
} rh_context;

/* Defined in execute.c */
int rh_execute(rh_context *ctx);

/* Defined in compile.c */
// rh_asm_global *rh_compile(rh_context *ctx);

/* Defined in memory.c */
// void rh_memman_init(rh_memman *man);
// void rh_memman_free(rh_memman *man);
// void rh_dump_memory_usage(FILE *fp, rh_memman *man);
// rh_size_t rh_malloc_type(rh_memman *man, rh_size_t size, rh_mem_type type);
void *rh_malloc(size_t size);
void rh_free(void *p);
void *rh_realloc(void *p, size_t size);

/* Defined in token.c */
void rh_token_init();
rh_token *rh_next_token(rh_context *ctx);
void rh_dump_token(FILE *fp, rh_token *token);

/* Defined in file.c */
int rh_nextchar(rh_context *ctx);
// void rh_ungetc(rh_file *file, int c);

/* Defined in error.c */
void rh_error(rh_context *ctx, rh_error_type type, rh_token *token, char *msg, ...);
void rh_error_dump(rh_error_context *err, FILE *fp);
#define E_FATAL(ctx,tkn,msg,...)	(rh_error((ctx),ETYPE_FATAL,(tkn),"%s:%d " msg,__FILE__,__LINE__,__VA_ARGS__+0))
#define E_ERROR(ctx,tkn,msg,...)	(rh_error((ctx),ETYPE_ERROR,(tkn),"%s:%d " msg,__FILE__,__LINE__,__VA_ARGS__+0))
#define E_WARNING(ctx,tkn,msg,...)	(rh_error((ctx),ETYPE_WARNING,(tkn),"%s:%d " msg,__FILE__,__LINE__,__VA_ARGS__+0))
#define E_NOTICE(ctx,tkn,msg,...)	(rh_error((ctx),ETYPE_NOTICE,(tkn),"%s:%d " msg,__FILE__,__LINE__,__VA_ARGS__+0))
#define E_INTERNAL(ctx,tkn,msg,...)	(rh_error((ctx),ETYPE_INTERNAL,(tkn),"%s:%d " msg,__FILE__,__LINE__,__VA_ARGS__+0))




