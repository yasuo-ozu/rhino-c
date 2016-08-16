#include "common.h"

struct /* ident_token_table */ {/*{{{*/
	rh_token_kind kind;
	char *ident;
} ident_token_table[] = {
	{TK_BOOL,		"_Bool"},		{TK_CHAR,		"char"},
	{TK_SHORT,		"short"},		{TK_INT,		"int"},
	{TK_LONG,		"long"},		{TK_SIGNED,		"signed"},
	{TK_UNSIGNED,	"unsigned"},	{TK_FLOAT,		"float"},
	{TK_DOUBLE,		"double"},		{TK_COMPLEX,	"_Complex"},
	{TK_IMAGINARY,	"_Imaginary"},	{TK_STRUCT,		"struct"},
	{TK_UNION,		"union"},		{TK_ENUM,		"enum"},
	{TK_VOLATILE,	"volatile"},	{TK_CONST,		"const"},
	{TK_RESTRICT,	"restrict"},	{TK_AUTO,		"auto"},
	{TK_EXTERN,		"extern"},		{TK_STATIC,		"static"},
	{TK_REGISTER,	"register"},	{TK_TYPEDEF,	"typedef"},
	{TK_VOID,		"void"},		{TK_IF,			"if"},
	{TK_ELSE,		"else"},		{TK_SWITCH,		"switch"},
	{TK_CASE,		"case"},		{TK_DEFAULT,	"default"},
	{TK_FOR,		"for"},			{TK_WHILE,		"while"},
	{TK_DO,			"do"},			{TK_GOTO,		"goto"},
	{TK_CONTINUE,	"continue"},	{TK_BREAK,		"break"},
	{TK_RETURN,		"return"},		{TK_INLINE,		"inline"},
	{TK_SIZEOF,		"sizeof"},
	{TK_NULL, 0}
};/*}}}*/

struct /*  multisymbol_token_table */ {/*{{{*/
	rh_token_kind kind;
	char *symbol;
} multisymbol_token_table[] = {
	/* 3 chars */
	{TK_ANDEQ,		"&&="},	{TK_OREQ,		"||="},
	/* 2 chars */
	{TK_AND,		"&&"},	{TK_OR,			"||"},	{TK_PLUSPLUS,	"++"},
	{TK_MINUSMINUS, "--"}, 	{TK_PLUSEQ,		"+="},	{TK_MINUSEQ,	"-="},
	{TK_MULEQ,		"*="}, 	{TK_DIVEQ,		"/="},	{TK_BITOREQ,	"|="},
	{TK_BITANDEQ,	"&="}, 	{TK_XOREQ,		"^="},	{TK_EQUAL,		"=="},
	{TK_NOTEQ,		"!="},	{TK_LE,			"<="},	{TK_GE,			">="},
	/* 1 chars */
	{TK_LPAREN,		"("}, {TK_RPAREN,		")"},
	{TK_LBRACE,		"{"}, {TK_RBRACE,		"}"},
	{TK_LBRACKET,	"["}, {TK_RBRACKET,		"]"},
	{TK_PLUS,		"+"}, {TK_MINUS,		"-"}, {TK_MUL,			"*"}, {TK_DIV,			"/"},
	{TK_ASSIGN,		"="}, {TK_SEMICOLON,	";"}, {TK_COLON,		":"}, {TK_SHARP,		"#"},
	{TK_COMMA,		","}, {TK_DOT,			"."}, {TK_MOD,			"%"}, {TK_NOT,			"!"},
	{TK_SNGQ,		"'"}, {TK_DBLQ,			"\""}, {TK_BITAND,		"&"}, {TK_BITOR,		"|"},
	{TK_XOR,		"^"}, {TK_LT,			"<"}, {TK_GT,			">"}, {TK_WHAT,			"?"},
	{TK_NULL,		0}
};/*}}}*/

void rh_dump_token(FILE *fp, rh_token token) { /*{{{*/
	int i;
	for (i = 0; ident_token_table[i].kind != TK_NULL; i++) {
		if (ident_token_table[i].kind == token.kind) {
			fprintf(fp, "KEYWORD:\t%s\n", ident_token_table[i].ident);
			return;
		}
	}
	for (i = 0; multisymbol_token_table[i].kind != TK_NULL; i++) {
		if (multisymbol_token_table[i].kind == token.kind) {
			fprintf(fp, "SYMBOL:\t%s\n", multisymbol_token_table[i].symbol);
			return;
		}
	}
	if (token.kind == TK_STRING) { fprintf(fp, "STRING\n"); return; }
	if (token.kind == TK_VAL) { 
		fprintf(fp, "VAL: %lld, %llf\n", token.val_int, token.val_float); 
		return;
	}
	if (token.kind == TK_IDENT) { fprintf(fp, "IDENT\n"); return; }
	if (token.kind == TK_NULL) { fprintf(fp, "NULL\n"); return; }
	fprintf(fp, "ERR\n");
} /*}}}*/

rh_token rh_next_token(rh_file *file) {/*{{{*/
	rh_token token = {TK_NULL, TYPE_NULL, 0, 0.0, NULL};
	int c = rh_getchar(file, 0), a, i, j, k;
	double d;
	char buf[MAX_TOKEN_LENGTH + 1];

	while (~c && isspace(c)) c = rh_getchar(file, 0);
	if(!~c) return token;

	if (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
		i = 0;
		do {
			if (i < MAX_TOKEN_LENGTH) buf[i++] = (char) c;
			c = rh_getchar(file, 0);
		} while (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' ||
				'0' <= c && c <= '9');
		buf[i] = '\0';
		for (i = 0; ident_token_table[i].kind != TK_NULL; i++) {
			if (strcmp(ident_token_table[i].ident,  buf) == 0) {
				token.kind = ident_token_table[i].kind; break;
			}
		}
		if (token.kind == TK_NULL) {
			token.kind = TK_IDENT;
		}
	} else if ('0' <= c && c <= '9') {
		token.kind = TK_VAL;
		token.type = TYPE_SINT;
		int is_16shin = 0;		// 16-shin flag
		if (c == '0') {
			c = rh_getchar(file, 0);
			if (c == 'x' || c == 'X') {
				is_16shin = 1;
				c = rh_getchar(file, 0);
				for (;;) {
					if ('0' <= c && c <= '9') i = c - '0';
					else if ('a' <= c && c <= 'f') i = c - 'a' + 10;
					else if ('A' <= c && c <= 'F') i = c - 'A' + 10;
					else break;
					token.val_int = token.val_int * 16 + i; 
					c = rh_getchar(file, 0);
				}
				token.val_float = (float) token.val_int;
			} else {	/* if (c == 'x' || c == 'X') */
				/* 8-shin number (or 0) */
				i = 0;
				while ('0' <= c && c <= '7') {
					i = i * 8 + (c - '0');
					c = rh_getchar(file, 0);
				}
				token.val_float = token.val_int = i;
			}
		} else {		/* if (c == '0') */
			i = 0;
			while ('0' <= c && c <= '9') {
				i = i * 10 + (c - '0');
				c = rh_getchar(file, 0);
			}
			token.val_float = token.val_int = i;
		}
		if (c == '.') {
			token.type = TYPE_DOUBLE;
			i = 10;
			c = rh_getchar(file, 0);
			while ('0' <= c && c <= '9') {
				token.val_float += (c - '0') / (double) i;
				i *= 10;
				c = rh_getchar(file, 0);
			}
		}
		if (is_16shin && (c == 'p' || c == 'P') || !is_16shin && (c == 'e' || c == 'E')) {
			c = rh_getchar(file, 0);
			j = c == '-' ? -1 : 1;
			a = 0;
			if (c == '-' || c == '+') a = c, c = rh_getchar(file, 0);
			if ('0' <= c && c <= '9') {
				i = 0;
				do {
					i = i * 10 + (c - '0');
					c = rh_getchar(file, 0);
				} while ('0' <= c && c <= '9');
				d = j < 0 ? 0.1 : 10.0;
				for (; i; i--) {
					token.val_int *= d;
					token.val_float *= d;
				}
				token.type = TYPE_DOUBLE;
			} else {
				fprintf(stderr, "err: Value power must be integer\n");
				exit(1);
				if (a) {
					rh_ungetc(file, c);
					c = a;
				}
			}
		}
		for (;;) {
			if (c == 'l' || c == 'L') {
				a = rh_getchar(file, 0);
				if ((a == 'l' || a == 'L') && token.type == TYPE_SINT) {
					token.type = TYPE_SLLONG;
				} else {
					rh_ungetc(file, a);
					if (token.type == TYPE_SINT) token.type = TYPE_SLONG;
					else if (token.type == TYPE_DOUBLE) token.type = TYPE_LDOUBLE;
					else {
						fprintf(stderr, "err: Missing in flag l\n");
						exit(1);
					}
				}
			}else if ((c == 'u' || c == 'U') && (token.type & 1)) {
				if (token.type & 16) {
					fprintf(stderr, "err: Missing in flag u\n");
					exit(1);
				} else {
					token.type &= 16;
				}
			} else if ((c == 'f' || c == 'F') && token.type == TYPE_DOUBLE) {
				token.type = TYPE_FLOAT;
			} else break;
			c = rh_getchar(file, 0);
		}
		if ('0' <= c && c <= '9' || c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z') {
			fprintf(stderr, "err: Missing in flag\n");
			exit(1);
		}
	} else if (c == '"') {
		// TODO:
	} else if (c == '\'') {
		// TODO:
	} else {
		for (i = 0; multisymbol_token_table[i].kind != 0; i++) {
			for (j = 0, k = 0; ~c && multisymbol_token_table[i].symbol[j] == c; j++) {
				buf[k++] = c; c = rh_getchar(file, 0);
			}
			if (multisymbol_token_table[i].symbol[j] == 0) {
				token.kind = multisymbol_token_table[i].kind; break;
			} else {
				/* TODO: This seems TOO SLOW */
				buf[k++] = c;
				while (k > 1) rh_ungetc(file, buf[--k]);
				if (k == 1) c = *buf;
			}
		}
		if (multisymbol_token_table[i].kind == TK_NULL) {
			fprintf(stderr, "Invalid char: %c\n", c);
			exit(1);
			// to escape the error:
			rh_ungetc(file, c);
		}
	}

	rh_ungetc(file, c);
	if (file->dump_token) {
		rh_dump_token(stderr, token);
	}
	return token;
}/*}}}*/

/* vim: set foldmethod=marker : */
