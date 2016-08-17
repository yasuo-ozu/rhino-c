#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>


/* common settings */

#define MAX_FNAME	1024

/* types and structures */

#define MAX_FILE_DEPTH	40
#define MAX_UNGET_BUF	10

typedef struct rh_file {
	FILE *fp;
	int line;
	int ch;
	char name[MAX_FNAME];
	struct rh_file *parent;

	/* buffer for rh_ungetc() */
	int unget_buf_top;
	int unget_buf[MAX_UNGET_BUF];

	/* file-spicified flags */
	int dump_token;
} rh_file;

#define MAX_TOKEN_LENGTH	1024
#define MAX_KEYWORD_LENGTH	12

typedef enum {
	TK_NULL = 0,

	/* symbols */
	TK_LPAREN = '(', TK_RPAREN = ')',
	TK_LBRACE = '{', TK_RBRACE = '}',
	TK_LBRACKET = '[', TK_RBRACKET = ']',
	TK_PLUS = '+', TK_MINUS = '-', TK_MUL = '*', TK_DIV = '/',
	TK_ASSIGN = '=', TK_SEMICOLON = ';', TK_COLON = ':', TK_SHARP = '#',
	TK_COMMA = ',', TK_DOT = '.', TK_MOD = '%', TK_NOT = '!',
	TK_SNGQ = '\'', TK_DBLQ = '"', TK_BITAND = '&', TK_BITOR = '|',
	TK_XOR = '^', TK_LT = '<', TK_GT = '>', TK_WHAT = '?',
	TK_AND /* && */, TK_OR /* || */, TK_PLUSPLUS /* ++ */,
	TK_MINUSMINUS /* -- */, TK_PLUSEQ /* += */, TK_MINUSEQ /* -= */,
	TK_MULEQ /* *= */, TK_DIVEQ /* /= */, TK_BITOREQ /* |= */,
	TK_BITANDEQ /* &= */, TK_XOREQ /* ^= */, TK_EQUAL /* == */,
	TK_NOTEQ /* != */, TK_ANDEQ /* &&= */, TK_OREQ /* ||= */,
	TK_LE /* <= */, TK_GE /* >= */,

	/* literals */
	TK_STRING, TK_VAL,

	/* keywords */
	TK_BOOL /* _Bool */, TK_CHAR, TK_SHORT, TK_INT, TK_LONG,
	TK_SIGNED, TK_UNSIGNED, TK_FLOAT, TK_DOUBLE,
	TK_COMPLEX /* _Complex */, TK_IMAGINARY /* _Imaginary */,
	TK_STRUCT, TK_UNION, TK_ENUM, TK_VOLATILE, TK_CONST,
	TK_RESTRICT, TK_AUTO, TK_EXTERN, TK_STATIC, TK_REGISTER,
	TK_TYPEDEF, TK_VOID, TK_IF, TK_ELSE, TK_SWITCH, TK_CASE,
	TK_DEFAULT, TK_FOR, TK_WHILE, TK_DO, TK_GOTO, TK_CONTINUE,
	TK_BREAK, TK_RETURN, TK_INLINE, TK_SIZEOF,

	/* OTHER */
	TK_IDENT

} rh_token_kind;

typedef enum {
	TYPE_NULL	= 0,	// 0000 0000
	TYPE_SINT	= 1, 	// 0000 0001
	TYPE_SCHAR	= 5, 	// 0000 0101
	TYPE_SSHORT = 9,	// 0000 1001
	TYPE_SLONG	= 1, 	// 0000 0001
	TYPE_SLLONG	= 33, 	// 0010 0001
	TYPE_UINT	= 17, 	// 0001 0001
	TYPE_UCHAR	= 21, 	// 0001 0101
	TYPE_USHORT	= 25,	// 0001 1001
	TYPE_ULONG	= 17, 	// 0001 0001
	TYPE_ULLONG	= 49, 	// 0011 0001
	TYPE_FLOAT	= 10,	// 0000 1010 
	TYPE_DOUBLE	= 2,	// 0000 0010 
	TYPE_LDOUBLE= 10,	// 0010 0010 
	TYPE_COMPLEX= 64,
	TYPE_IMAGINARY=128
	
} rh_type;

typedef struct {
	rh_token_kind kind;
	rh_type type;
	long long val_int;
	long double val_float;
	void *val_pointer;
	int line1, ch1, line2, ch2;
} rh_token;

#define MEMORY_TANK_MAX	20
#define MEMORY_SIZ		((rh_size_t) 0xFFFFFFFF)
#define MEMORY_TEXT_START	0x400000
#define MEMORY_DATA_START	0x800000 /* TODO: Is this efficient enough? */
#define MEMORY_HEAP_START	0xC00000 /* TODO: Is this efficient enough? */
#define MEMORY_MIN_TEXT_SIZE	0x10000
#define MEMORY_MIN_DATA_SIZE	0x10000
#define MEMORY_MIN_HEAP_SIZE	0x10000
#define MEMORY_MIN_STACK_SIZE	0x10000

typedef enum {
	MTYPE_NULL = 0, MTYPE_HEAP = 16, MTYPE_STACK = 8, MTYPE_DATA = 5, MTYPE_TEXT = 3, MTYPE_RUNNABLE = 2, MTYPE_RO =1
} rh_mem_type;

typedef unsigned int rh_size_t;

typedef struct {
	struct {
		rh_mem_type type;
		rh_size_t begin, end, size;
		void *ptr;
	} tank[MEMORY_TANK_MAX];
	int tanks;
	rh_size_t pbreak;	/* changed from rh_malloc() */
	rh_size_t stack;	/* changed from rh_malloc() */
	rh_size_t text, text_top;
	rh_size_t data, data_top;		/* BSS */
	rh_size_t heap;
} rh_memman;

typedef struct rh_asm_exp {
	struct rh_asm_exp *arg1, *arg2, *arg3;
	rh_token token;
} rh_asm_exp;
/*
typedef struct {
	
} rh_entry_ident;
*/
typedef struct {
	rh_asm_exp *exp;
} rh_asm_global;

typedef struct {
	rh_token token;
} rh_compile_context;

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

typedef struct {
	rh_memman memman;
	rh_file file;
	rh_compile_context compile;
	rh_error_context error;
} rh_context;

/* Defined in execute.c */
int rh_execute(rh_asm_global *global);

/* Defined in compile.c */
rh_asm_global rh_compile(rh_context *ctx);

/* Defined in memory.c */
void rh_memman_init(rh_memman *man);
void rh_memman_free(rh_memman *man);
void rh_dump_memory_usage(FILE *fp, rh_memman *man);
rh_size_t rh_malloc_type(rh_memman *man, rh_size_t size, rh_mem_type type);

/* Defined in token.c */
void rh_token_init();
rh_token rh_next_token(rh_context *ctx);
void rh_dump_token(FILE *fp, rh_token token);

/* Defined in file.c */
int rh_getchar(rh_context *ctx, int in_literal);
void rh_ungetc(rh_file *file, int c);

