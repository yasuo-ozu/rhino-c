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

enum {/*{{{*/
	CP_CAPITAL		= 1, CP_LITERAL		= 2, CP_8DIGIT		= 4,
	CP_10DIGIT		= 8, CP_16DIGIT		= 16, CP_SPACE		= 32,
	CP_IDENT_FIRST	= 64, CP_IDENT		= 128, CP_L			= 256,
	CP_F			= 512, CP_P			= 1024, CP_E			= 2048,
	CP_U			= 4096, CP_X		= 8192
} ctbl[0xFF];/*}}}*/

void rh_token_init() {/*{{{*/
	int c;
	for (c = 0; c < 0xFF; c++) {
		if ('A' <= c && c <= 'Z')	ctbl[c] |= CP_CAPITAL;
		if ('a' <= c && c <= 'z')	ctbl[c] |= CP_LITERAL;
		if ('0' <= c && c <= '7')	ctbl[c] |= CP_8DIGIT;
		if ('0' <= c && c <= '9')	ctbl[c] |= CP_10DIGIT;
		if ('0' <= c && c <= '9' || 'A' <= c && c <= 'F' || 'a' <= c && c <= 'f')
									ctbl[c] |= CP_16DIGIT;
		if (isspace(c))				ctbl[c] |= CP_SPACE;
		if (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z')
									ctbl[c] |= CP_IDENT_FIRST;
		if (c == '_' || 'a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || '0' <= c && c <= '9')
									ctbl[c] |= CP_IDENT;
	}
	ctbl['L'] = ctbl['l'] = CP_L;
	ctbl['F'] = ctbl['f'] = CP_F;
	ctbl['P'] = ctbl['p'] = CP_P;
	ctbl['E'] = ctbl['e'] = CP_E;
	ctbl['U'] = ctbl['u'] = CP_U;
	ctbl['X'] = ctbl['x'] = CP_X;
}/*}}}*/

rh_token rh_next_token(rh_file *file) {/*{{{*/
	rh_token token = {TK_NULL, TYPE_NULL, 0, 0.0, NULL};
	int c = rh_getchar(file, 0), a, i, j, k;
	double d;
	char buf[MAX_TOKEN_LENGTH + 1];

	while (~c && (ctbl[c] & CP_SPACE)) c = rh_getchar(file, 0);
	if(!~c) return token;

	if (ctbl[c] & CP_IDENT_FIRST) {
		i = 0;
		do {
			if (i < MAX_TOKEN_LENGTH) buf[i++] = (char) c;
			c = rh_getchar(file, 0);
		} while (ctbl[c] & CP_IDENT);
		buf[i] = '\0';
		for (i = 0; ident_token_table[i].kind != TK_NULL; i++) {
			if (strcmp(ident_token_table[i].ident,  buf) == 0) {
				token.kind = ident_token_table[i].kind; break;
			}
		}
		if (token.kind == TK_NULL) {
			token.kind = TK_IDENT;
		}
	} else if (ctbl[c] & CP_10DIGIT) {
		token.kind = TK_VAL;
		token.type = TYPE_SINT;
		int is_16shin = 0;		// 16-shin flag
		if (c == '0') {
			c = rh_getchar(file, 0);
			if (ctbl[c] & CP_X) {
				is_16shin = 1;
				c = rh_getchar(file, 0);
				for (;;) {
					if (ctbl[c] & CP_10DIGIT) i = c - '0';
					else if (ctbl[c] & CP_16DIGIT) 
						i = c - (ctbl[c] & CP_CAPITAL ? 'A' : 'a') + 10;
					else break;
					token.val_int = token.val_int * 16 + i; 
					c = rh_getchar(file, 0);
				}
				token.val_float = (float) token.val_int;
			} else {
				/* 8-shin number (or 0) */
				i = 0;
				while (ctbl[c] & CP_8DIGIT) {
					i = i * 8 + (c - '0');
					c = rh_getchar(file, 0);
				}
				token.val_float = token.val_int = i;
			}
		} else {
			i = 0;
			while (ctbl[c] & CP_10DIGIT) {
				i = i * 10 + (c - '0');
				c = rh_getchar(file, 0);
			}
			token.val_float = token.val_int = i;
		}
		if (c == '.') {
			token.type = TYPE_DOUBLE;
			i = 10;
			c = rh_getchar(file, 0);
			while (ctbl[c] & CP_10DIGIT) {
				token.val_float += (c - '0') / (double) i;
				i *= 10;
				c = rh_getchar(file, 0);
			}
		}
		if (is_16shin && ctbl[c] & CP_P || !is_16shin && ctbl[c] & CP_E) {
			c = rh_getchar(file, 0);
			j = c == '-' ? -1 : 1;
			a = 0;
			if (c == '-' || c == '+') a = c, c = rh_getchar(file, 0);
			if (ctbl[c] & CP_10DIGIT) {
				i = 0;
				do {
					i = i * 10 + (c - '0');
					c = rh_getchar(file, 0);
				} while (ctbl[c] & CP_10DIGIT);
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
			if (ctbl[c] & CP_L) {
				a = rh_getchar(file, 0);
				if (ctbl[a] & CP_L && token.type == TYPE_SINT) {
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
			}else if (ctbl[c] & CP_U && token.type & 1) {
				if (token.type & 16) {
					fprintf(stderr, "err: Missing in flag u\n");
					exit(1);
				} else {
					token.type &= 16;
				}
			} else if (ctbl[c] & CP_F && token.type == TYPE_DOUBLE) {
				token.type = TYPE_FLOAT;
			} else break;
			c = rh_getchar(file, 0);
		}
		if (ctbl[c] & CP_IDENT) {
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
