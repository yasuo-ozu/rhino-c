#include "common.h"

char *symbol_with_multipul_chars[] = {/*{{{*/
	/* 3 chars */
	"&&=", "||=", "<<=", ">>=",
	/* 2 chars */
	"&&", "||", "++", "--", "<<", ">>",  "+=", "-=", "*=", "/=", 
	"|=", "&=", "^=", "==", "!=", "<=", ">=", "->", 0
};/*}}}*/

void rh_dump_token(FILE *fp, rh_token *token) { /*{{{*/
	static char *token_type_name[] = {
		"NULL", "IDENT", "NUMERIC", "SYMBOL", "CHAR", "STRING"
	};
	char *c;
	fprintf(fp, "token ( type = %s, text = ",
		token_type_name[token->type]);
	for(c = token->file_begin; c < token->file_end; c++) fputc(*c, fp);
	fprintf(fp, ")\n");
} /*}}}*/

void rh_token_init() {/*{{{*/
	int c;
	char symbols[] = "!\"#$%&'()*+,-./:;<=>?p[\\]^`{|}~", *s;
	for (c = 0; c < 0xFF; c++) {
		ctbl[c] = 0;
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
	for (s = symbols; *s; s++) ctbl[*s] |= CP_SYMBOL;
}/*}}}*/

rh_token *rh_next_token(rh_context *ctx) {/*{{{*/
	int i, j;
	while (rh_nextchar(ctx) == 1 || ctbl[*ctx->ch] & CP_SPACE) ctx->ch++;
	rh_token *token = malloc(sizeof(rh_token));
	token->type = TKN_NULL;
	token->file = ctx->file;
	token->file_begin = ctx->ch;
	token->var = NULL;
	if (*ctx->ch) {
		if (ctbl[*ctx->ch] & CP_IDENT_FIRST) {
			token->type = TKN_IDENT;
			while (ctbl[*ctx->ch] & CP_IDENT) ctx->ch++;
		} else if (ctbl[*ctx->ch] & CP_10DIGIT) {	/* _A-Za-z0-9 */
			long long intval = 0; long double dblval = 0.0;
			int is_hexadecimal = 0, is_dbl = 0;
			rh_type *type = rh_create_type(ctx);
			token->type = TKN_NUMERIC;
			if (*ctx->ch == '0') {
				ctx->ch++;
				if (*ctx->ch == 'x' || *ctx->ch == 'X') {
					is_hexadecimal = 1;
					ctx->ch++;
					for (;;) {
						int dgt;
						if (ctbl[*ctx->ch] & CP_10DIGIT) dgt = *ctx->ch - '0';
						else if (ctbl[*ctx->ch] & CP_16DIGIT) 
							dgt = *ctx->ch - (ctbl[*ctx->ch] & CP_CAPITAL ? 'A' : 'a') + 10;
						else break;
						intval = intval * 16 + dgt; 
						ctx->ch++;
					}
				} else {	// octadecimal number or 0
					while (ctbl[*ctx->ch] & CP_8DIGIT)
						intval = intval * 8 + (*ctx->ch++ - '0');
				}
			} else {
				while (ctbl[*ctx->ch] & CP_10DIGIT)
					intval = intval * 10 + (*ctx->ch++ - '0');
			}
			if (*ctx->ch == '.') {
				is_dbl = 1; dblval = (long double) intval;
				double pwr = 10.0;
				ctx->ch++;
				while (ctbl[*ctx->ch] & CP_10DIGIT) {
					dblval += (*ctx->ch++ - '0') / pwr;
					pwr *= 10;
				}
			}
			if (is_hexadecimal && (*ctx->ch == 'P' || *ctx->ch == 'p') 
				|| !is_hexadecimal && (*ctx->ch == 'E' || *ctx->ch == 'e')) {
				is_dbl = 1; ctx->ch++;
				double pwr = *ctx->ch == '-' ? 0.1 : 10.0;
				if (*ctx->ch == '-' || *ctx->ch == '+') ctx->ch++;
				if (ctbl[*ctx->ch] & CP_10DIGIT) {
					int p = 0;
					do {
						p = p * 10 + (*ctx->ch++ - '0');
					} while (ctbl[*ctx->ch++] & CP_10DIGIT);
					for (; p; p--) dblval *= pwr;
				} else {
					E_ERROR(ctx, ctx->token, "Value power must be integer");
				}
			}
			int unsigned_count = 0, long_count = 0, size = 4, float_count = 0;
			for (;;) {
				if (*ctx->ch == 'l' || *ctx->ch == 'L') long_count++;
				else if (*ctx->ch == 'u' || *ctx->ch == 'U') unsigned_count++;
				else if (*ctx->ch == 'f' || *ctx->ch == 'F') float_count++;
				else break;
				ctx->ch++;
			}
			if (ctbl[*ctx->ch] & CP_IDENT) {
				E_ERROR(ctx, ctx->token, "Missing in flag");
			}
			if (is_dbl) {
				type->specifier = SP_FLOATING;
				type->sign = 1;
				if (long_count > 1) { E_ERROR(ctx, ctx->token, "Missing in flag L"); }
				if (float_count > 1 || float_count && long_count) {
					E_ERROR(ctx, ctx->token, "Missing in flag F");
				}
				if (unsigned_count) { E_ERROR(ctx, ctx->token, "Missing in flag U"); }
				type->size = float_count ? 4 : long_count ? 16 : 8;
			} else {
				type->specifier = SP_NUMERIC;
				if (long_count > 2) { E_ERROR(ctx, ctx->token, "Missing in flag L"); }
				if (float_count) { E_ERROR(ctx, ctx->token, "Missing in flag F"); }
				if (unsigned_count > 1) {
					E_ERROR(ctx, ctx->token, "Missing in flag U");
					unsigned_count = 1;
				}
				type->size = long_count >= 2 ? 8 : 4;
				type->sign = unsigned_count;
			}
			rh_variable *var = rh_create_variable(ctx, type);
			if (is_dbl) {
				if (type->size == 16) *((long double *) var->memory) = dblval;
				else if (type->size == 8) *((double *) var->memory) = dblval;
				else if (type->size == 4) *((float *) var->memory) = dblval;
			} else {
				if (type->sign) {
					if (type->size == 8) *((long long *) var->memory) = intval;
					if (type->size == 4) *((signed *) var->memory) = intval;
					if (type->size == 2) *((short *) var->memory) = intval;
					if (type->size == 1) *((signed char *) var->memory) = intval;
				} else {
					if (type->size == 8) *((unsigned long long *) var->memory) = intval;
					if (type->size == 4) *((unsigned *) var->memory) = intval;
					if (type->size == 2) *((unsigned short *) var->memory) = intval;
					if (type->size == 1) *((unsigned char *) var->memory) = intval;
				}
			}
			token->var = var;
		} else if (*ctx->ch == '"' || *ctx->ch == '\'') {
			char a = *ctx->ch;		/* reserve starti signedhng s signed ymbol */
			rh_type *type = rh_create_type(ctx);
			type->specifier = SP_NUMERIC;
			type->size = 1; type->sign = 1;
			if (a == '"') {
				rh_type *type2 = rh_create_type(ctx);
				type2->child = type;
				type2->is_pointer = 1;
				type2->size = 4;
				type = type2;
			}
			rh_variable *var = rh_create_variable(ctx, type);
			token->type = a == '"' ? TKN_STRING : TKN_CHAR;
			ctx->ch++;
			// TODO: エスケープ処理
			// TODO: 文字列の処理
			*var->memory = (unsigned char)0;
			while (*ctx->ch && *ctx->ch != a) {
				*var->memory = *ctx->ch;
				ctx->ch++;
				// TODO: 複数文字をintとして解釈
			}
			ctx->ch++;
		} else {
			token->type = TKN_SYMBOL;
			for (i = 0; symbol_with_multipul_chars[i]; i++) {
				for (j = 0; symbol_with_multipul_chars[i][j] == ctx->ch[j]; ) {
					if (symbol_with_multipul_chars[i][++j] == 0) break;
				}
				if (symbol_with_multipul_chars[i][j] == 0) {
					ctx->ch += j - 1;
					break;
				}
			}
			ctx->ch++;
		}
	}
	token->file_end = ctx->ch;
	if (ctx->file->dump_token) {
		rh_dump_token(stderr, token);
	}
	return token;
}/*}}}*/


/* vim: set foldmethod=marker : */
