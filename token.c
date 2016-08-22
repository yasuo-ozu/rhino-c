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
	printf("token ( type = %s, line = (%d, %d), ch = (%d, %d), byte = (%d, %d), text = ",
		token_type_name[token->type], token->line1, token->line2, token->ch1, 
		token->ch2, token->byte1, token->byte);
	for(c = token->file_begin; c < token->file_end; c++) fputc(fp, *c);
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

size_t get_token_min_size() {/*{{{*/
	rh_token *token;
	size_t ret;
	static size_t token_min_size = 0;
	if (token_min_size) return token_min_size;
	token = malloc(0xFF);	/* 0xFF is large enough? */
	ret = (unsigned char *) &token->text - (unsigned char *) token;
	free(token);
	return token_min_size = ret;
}/*}}}*/

void rh_next_token(rh_context *ctx) {/*{{{*/
	int i, j, k;
	char text[256], c = *ctx->ch;
	double d;
	rh_token *token = malloc(sizeof(rh_token));
	token->type = TKN_NULL;
	token->file = ctx->file;

	while (c && (ctbl[c] & CP_SPACE)) c = rh_getchar(ctx, 0);
	token->file_begin = ctx->ch;
	if (c) {
		if (ctbl[c] & CP_IDENT_FIRST) {
			token->type = TKN_IDENT;
			do c = rh_getchar(ctx, 0); while (c && ctbl[c] & CP_IDENT);
		} else if (ctbl[c] & CP_10DIGIT) {	/* _A-Za-z0-9 */
			token->type = TKN_NUMERIC;
			token->literal.intval = 0;
			token->literal.dblval = 0.0;
			token->literal.type = TYPE_INT;
			int is_hexadecimal = 0;
			if (c == '0') {
				c = rh_getchar(ctx, 0);
				if (c == 'x' || c == 'X') {
					is_hexadecimal = 1;
					c = rh_getchar(ctx, 0);
					for (;;) {
						if (ctbl[c] & CP_10DIGIT) val_ll = c - '0';
						else if (ctbl[c] & CP_16DIGIT) 
							val_ll = c - (ctbl[c] & CP_CAPITAL ? 'A' : 'a') + 10;
						else break;
						token->literal.intval = token->literal.intval * 16 + val_ll; 
						c = rh_getchar(ctx, 0);
					}
					token->literal.dblval = (long double) token->literal.intval;
				} else {	// octadecimal number or 0
					while (ctbl[c] & CP_8DIGIT) {
						token->literal.intval = token->literal.intval * 8 + (c - '0');
						c = rh_getchar(ctx, 0);
					}
				}
			} else {
				while (ctbl[c] & CP_10DIGIT) {
					token->literal.intval = token->literal.intval * 10 + (c - '0');
					c = rh_getchar(ctx, 0);
				}
			}
			if (c == '.') {
				token->literal.type = TYPE_DOUBLE;
				token->literal.dblval = (long double) exp->literal.intval;
				val_ll = 10;
				c = rh_getchar(ctx, 0);
				while (ctbl[c] & CP_10DIGIT) {
					exp->literal.dblval += (c - '0') / (double) val_ll;
					val_ll *= 10;
					c = rh_getchar(ctx, 0);
				}
			}
			if (is_hexadecimal && (c == 'P' || c == 'p') || !is_hexadecimal && (c == 'E' || c == 'e')) {
				token->literal.type = TYPE_DOUBLE;
				c = rh_getchar(ctx, 0);
				val_ld = c == '-' ? 0.1 : 10.0;
				if (c == '-' || c == '+') c = rh_getchar(ctx, 0);
				if (ctbl[c] & CP_10DIGIT) {
					val_ll = 0;
					do {
						val_ll = val_ll * 10 + (c - '0');
						c = rh_getchar(ctx, 0);
					} while (ctbl[c] & CP_10DIGIT);
					for (; val_ll; val_ll--) {
						token->literal.intval *= val_ld;
						token->literal.dblval *= val_ld;
					}
				} else {
					E_ERROR(ctx, ctx->token, "Value power must be integer\n");
				}
			}/*
			for (;;) {
				if (ctbl[c] & CP_L) {
					a = rh_getchar(ctx, 0);
					if (ctbl[a] & CP_L && token.type == TYPE_SINT) {
						token.type = TYPE_SLLONG;
					} else {
						rh_ungetc(&ctx->file, a);
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
				c = rh_getchar(ctx, 0);
			}*/
			if (ctbl[c] & CP_IDENT) {
				E_ERROR(ctx, ctx->token, "Missing in flag\n");
			}

#if 0
			// do {
			// 	if (token_min_size + count + 1 > token_size) {
			// 		token_size = token_size + 128;
			// 		token = realloc(token, token_size);
			// 	}
			// 	token->text[count++] = (char) c;
			// 	if (token->type == TKN_NUMERIC && (c == 'e' || c == 'E' || c == 'p' || c == 'P')) {
			// 		c = rh_getchar(ctx, 0);
			// 		if (c == '+' || c == '-') {
			// 			token->text[count++] = (char) c;
			// 			c = rh_getchar(ctx, 0);
			// 		}
			// 	} else c = rh_getchar(ctx, 0);
			// 	if (token->type == TKN_NUMERIC && c == '.') {
			// 		token->text[count++] = (char) c;
			// 		c = rh_getchar(ctx, 0);
			// 	}
			// } while (ctbl[c] & CP_IDENT);
#endif
		} else if (c == '"' || c == '\'') {
			// a = c;		/* reserve starting symbol */
			// token->type = a == '"' ? TKN_STRING : TKN_CHAR;
			// token->text[count++] = a;
			// while (c == a) {
			// 	do {
			// 		if (token_min_size + count + 2 > token_size) {
			// 			token_size = token_size + 128;
			// 			token = realloc(token, token_size);
			// 		}
			// 		c = rh_getchar(ctx, 0);
			// 		if (c == '\\') {
			// 			c = rh_getchar(ctx, 0);
			// 			if (c != a) token->text[count++] = '\\';
			// 		}
			// 		token->text[count++] = c;
			// 	} while (c != a && c != '\n' && c != EOF);
			// 	if (c != a) E_ERROR(ctx, 0, "needs %c", a);
			// 	do c = rh_getchar(ctx, 0); while (c != EOF && (ctbl[c] & CP_SPACE));
			// }
			// token->text[count++] = a;
			// token->text[count] = '\0';
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
	if (ctx->file->dump_token) {
		rh_dump_token(stderr, token);
	}
	token->file_end = ctx->ch;
	token->prev = ctx->token;
	ctx->token = token;
}/*}}}*/

/* vim: set foldmethod=marker : */
