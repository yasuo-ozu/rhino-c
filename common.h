#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


/* common settings */

#define MAX_FNAME	1024

/* Defined in main.c */


/* Defined in file.c */

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

int rh_getchar(rh_file *file, int in_literal);
void rh_ungetc(rh_file *file, int c);

/* Defined in token.c */

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
	long long val_float;
	void *val_pointer;
} rh_token;

rh_token rh_next_token(rh_file *file);
void rh_dump_token(FILE *fp, rh_token token);

/* Defined in compile.c */




