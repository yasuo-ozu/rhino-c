#include "common.h"

struct {
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
};

struct {
	rh_token_kind kind;
	char *symbol;
} multisymbol_token_table[] = {
	{TK_AND,		"&&"},	{TK_OR,			"||"},	{TK_PLUSPLUS,	"++"},
	{TK_MINUSMINUS, "--"}, 	{TK_PLUSEQ,		"+="},	{TK_MINUSEQ,	"-="},
	{TK_MULEQ,		"*="}, 	{TK_DIVEQ,		"/="},	{TK_BITOREQ,	"|="},
	{TK_BITANDEQ,	"&="}, 	{TK_XOREQ,		"^="},	{TK_EQUAL,		"=="},
	{TK_NOTEQ,		"!="}, 	{TK_ANDEQ,		"&&="},	{TK_OREQ,		"||="},
	{TK_LE,			"<="},	{TK_GE,			">="},
	{TK_NULL,		0}
};
	

void rh_dump_token(FILE *fp, rh_token token) {
	int i;
	for (i = 0; ; i++) {
		if (ident_token_table[i].kind == token.kind) {
			fprintf(fp, "KEYWORD:\t%s\n", ident_token_table[i].ident);
			return;
		}
	}
	for (i = 0; ; i++) {
		if (multisymbol_token_table[i].kind == token.kind) {
			fprintf(fp, "SYMBOL:\t%s\n", multisymbol_token_table[i].symbol);
			return;
		}
	}
	if (token.kind == TK_STRING) {
		fprintf(fp, "STRING\n");
		return;
	}
	if (token.kind == TK_VAL) {
		fprintf(fp, "VAL\n");
		return;
	}
	if (token.kind == TK_IDENT) {
		fprintf(fp, "IDENT\n");
		return;
	}
	if (token.kind == TK_NULL) {
		fprintf(fp, "NULL\n");
		return;
	}
	fprintf(fp, "SYMBOL: %c\n", token.kind);
}


rh_token rh_next_token(rh_file *file) {
	rh_token token;


	if (file->dump_token) {
		rh_dump_token(stderr, token);
	}
	return token;
}

