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
	int i;
	printf("token ( type = %s, line = (%d, %d), ch = (%d, %d), byte = (%d, %d), text = %s\n",
		token_type_name[token->type], token->line1, token->line2, token->ch1, 
		token->ch2, token->byte1, token->byte2, token->text);
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

	size_t token_min_size = get_token_min_size(), token_size;
	int c = rh_getchar(ctx, 0), a, i, j, k, count = 0;
	double d;
	rh_token *token;

	token_size = token_min_size + 32;	/* Normally almost all tokens length < 32 */
	token = malloc(token_size);
	token->type = TKN_NULL;
	*token->text = '\0';
	token->file = ctx->file;
	token->line1	= token->line2	= token->file->line;
	token->ch1		= token->ch2	= token->file->ch;
	token->byte1	= token->byte2	= token->file->byte;

	while (c != EOF && (ctbl[c] & CP_SPACE)) c = rh_getchar(ctx, 0);
	if (c != EOF) {
		if (ctbl[c] & CP_IDENT) {	/* _A-Za-z0-9 */
			token->type = (ctbl[c] & CP_IDENT_FIRST) ? TKN_IDENT : TKN_NUMERIC;
			do {
				if (token_min_size + count + 1 > token_size) {
					token_size = token_size + 128;
					token = realloc(token, token_size);
				}
				token->text[count++] = (char) c;
				c = rh_getchar(ctx, 0);
			} while (ctbl[c] & CP_IDENT);	// TODO: 数値の指数に符号が入ってた時の処理
			token->text[count] = 0;
			token->text[count + 1] = 0;
			// token->text[count + 2] = 0;
			// token->text[count + 3] = 0;
		} else if (c == '"' || c == '\'') {
			a = c;		/* reserve starting symbol */
			token->type = a == '"' ? TKN_STRING : TKN_CHAR;
			token->text[count++] = a;
			while (c == a) {
				do {
					if (token_min_size + count + 2 > token_size) {
						token_size = token_size + 128;
						token = realloc(token, token_size);
					}
					c = rh_getchar(ctx, 0);
					if (c == '\\') {
						c = rh_getchar(ctx, 0);
						if (c != a) token->text[count++] = '\\';
					}
					token->text[count++] = c;
				} while (c != a && c != '\n' && c != EOF);
				if (c != a) E_ERROR(ctx, 0, "needs %c", a);
				do c = rh_getchar(ctx, 0); while (c != EOF && (ctbl[c] & CP_SPACE));
			}
			token->text[count++] = a;
			token->text[count] = '\0';
		} else {
			token->type = TKN_SYMBOL;
			for (i = 0; symbol_with_multipul_chars[i]; i++) {
				a = c;
				for (count = 0; c != EOF && symbol_with_multipul_chars[i][count] == c; count++) {
					if (token_min_size + count + 1 > token_size) {
						token_size = token_size + 128;
						token = realloc(token, token_size);
					}
					token->text[count] = c;
					if (c == '\0') break;
					c = rh_getchar(ctx, 0);
				}
				if (count) {
					while (count > 1) rh_ungetc(ctx->file, token->text[--count]);
					c = a; count = 0;
				} else break;
			}
			if (!count) {
				token->text[count++] = c;
				token->text[count] = '\0';
				c = rh_getchar(ctx, 0);
			}
		}
		rh_ungetc(ctx->file, c);
	}
	if (ctx->file->dump_token) {
		rh_dump_token(stderr, token);
	}
	token->prev = ctx->token;
	ctx->token = token;
}/*}}}*/
			/*
			token.kind = TK_VAL;
			token.type = TYPE_SINT;
			int is_16shin = 0;		// 16-shin flag
			if (c == '0') {
				c = rh_getchar(ctx, 0);
				if (ctbl[c] & CP_X) {
					is_16shin = 1;
					c = rh_getchar(ctx, 0);
					for (;;) {
						if (ctbl[c] & CP_10DIGIT) i = c - '0';
						else if (ctbl[c] & CP_16DIGIT) 
							i = c - (ctbl[c] & CP_CAPITAL ? 'A' : 'a') + 10;
						else break;
						token.val_int = token.val_int * 16 + i; 
						c = rh_getchar(ctx, 0);
					}
					token.val_float = (float) token.val_int;
				} else {
					// 8-shin number (or 0) 
					i = 0;
					while (ctbl[c] & CP_8DIGIT) {
						i = i * 8 + (c - '0');
						c = rh_getchar(ctx, 0);
					}
					token.val_float = token.val_int = i;
				}
			} else {
				i = 0;
				while (ctbl[c] & CP_10DIGIT) {
					i = i * 10 + (c - '0');
					c = rh_getchar(ctx, 0);
				}
				token.val_float = token.val_int = i;
			}
			if (c == '.') {
				token.type = TYPE_DOUBLE;
				i = 10;
				c = rh_getchar(ctx, 0);
				while (ctbl[c] & CP_10DIGIT) {
					token.val_float += (c - '0') / (double) i;
					i *= 10;
					c = rh_getchar(ctx, 0);
				}
			}
			if (is_16shin && ctbl[c] & CP_P || !is_16shin && ctbl[c] & CP_E) {
				c = rh_getchar(ctx, 0);
				j = c == '-' ? -1 : 1;
				a = 0;
				if (c == '-' || c == '+') a = c, c = rh_getchar(ctx, 0);
				if (ctbl[c] & CP_10DIGIT) {
					i = 0;
					do {
						i = i * 10 + (c - '0');
						c = rh_getchar(ctx, 0);
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
						rh_ungetc(&ctx->file, c);
						c = a;
					}
				}
			}
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
			}
			if (ctbl[c] & CP_IDENT) {
				fprintf(stderr, "err: Missing in flag\n");
				exit(1);
			}
			*/

/* vim: set foldmethod=marker : */
