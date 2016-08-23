#include "common.h"

int token_cmp(rh_token *token, char *ident) {/*{{{*/
	char *c = token->file_begin;
	while (c < token->file_end) {
		if (!*ident || *c != *ident) return 0;
		c++; ident++;
	}
	return *ident ? 0 : 1;
}/*}}}*/

int token_cmp_skip(rh_context *ctx, char *ident) {
	int ret = token_cmp(ctx->token, ident);
	if (ret) ctx->token = ctx->token->next;
	return ret;
}

/* type: 2: pre-op or conditional operator, 1: biary operator, 4: post operator */
int get_priority(rh_token *token, int type) {/*{{{*/
	struct {
		char *symbol; int priority; int type;
	} priority_table[] = {
		{"++", 1, 4}, {"--", 1, 4}, {"++", 2, 2}, {"--", 2, 2}, {"+", 2, 2},
		{"-", 2, 2}, {"~", 2, 2}, {"!", 2, 2}, {"*", 4, 1}, {"/", 4, 1},
		{"%", 4, 1}, {"+", 5, 1}, {"-", 5, 1}, {"<<", 6, 1}, {">>", 6, 1},
		{"<", 7, 1}, {"<=", 7, 1}, {">", 7, 1}, {">=", 7, 1}, {"==", 8, 1},
		{"!=", 8, 1}, {"&", 9, 1}, {"^", 10, 1}, {"|", 11, 1}, {"&&", 12, 1},
		{"&&", 13, 1}, {"&&", 14, 1}, {"=", 15, 1}, {",", 16, 1}, {0, 0, 1}
	};
	int i;
	for (i = 0; priority_table[i].symbol; i++) {
		if (priority_table[i].type & type && token_cmp(token, priority_table[i].symbol)) {
			return priority_table[i].priority;
		}
	}
	return -1;
}/*}}}*/

void error_with_token(rh_context *ctx, char *require, char *after) {/*{{{*/
	if (!token_cmp_skip(ctx, require)) {
		if (after) {
			E_ERROR(ctx, ctx->token, "requires '%s' after '%s'", require, after);
		} else {
			E_ERROR(ctx, ctx->token, "requires '%s'", require);
		}
	}
}/*}}}*/

/* When enabled==0, supress side effects. */
void rh_execute_expression(rh_context *ctx, int *ret, int enabled);
void rh_execute_expression_internal(rh_context *ctx, int *ret, int priority, int enabled) {/*{{{*/
	int has_op = 0, i, j;
	rh_token *tkn;
	if (priority == 0) {
		if (ctx->token->type == TKN_NUMERIC) {
			*ret = ctx->token->literal.intval;
			ctx->token = ctx->token->next;
		} else if (token_cmp_skip(ctx, "(")) {
			rh_execute_expression(ctx, ret, enabled);
			error_with_token(ctx, ")", 0);
		} else {
			E_ERROR(ctx, 0, "Invalid endterm");
			ctx->token = ctx->token->next;
			*ret = 1;
		}
	} else {
		if (get_priority(ctx->token, 2) == priority) {
			tkn = ctx->token;
			ctx->token = ctx->token->next;
			rh_execute_expression_internal(ctx, ret, priority, enabled);
			if (token_cmp(tkn, "+")) *ret = *ret;
			else if (token_cmp(tkn, "-")) *ret = -*ret;
			else if (token_cmp(tkn, "!")) *ret = !*ret;
			else if (token_cmp(tkn, "~")) *ret = ~*ret;
			else if (token_cmp(tkn, "++")) *ret += 1;
			 else if (token_cmp(tkn, "--")) *ret -= 1;
			else {
				fprintf(stderr, "Not implemented ");
				rh_dump_token(stdout, tkn);
			}
		} else {
			rh_execute_expression_internal(ctx, ret, priority - 1, enabled);
			while (get_priority(ctx->token, 1) == priority) {
				tkn = ctx->token;
				ctx->token = ctx->token->next;
				rh_execute_expression_internal(ctx, &i, priority - 1, enabled);
				if (token_cmp(tkn, "+")) *ret += i;
				else if (token_cmp(tkn, "-")) *ret -= i;
				else if (token_cmp(tkn, "*")) *ret *= i;
				else if (token_cmp(tkn, "/")) *ret /= i;
				else if (token_cmp(tkn, "%")) *ret %= i;
				else if (token_cmp(tkn, "<<")) *ret <<= i;
				else if (token_cmp(tkn, ">>")) *ret >>= i;
				else if (token_cmp(tkn, "<")) *ret = *ret < i;
				else if (token_cmp(tkn, "<=")) *ret = *ret <= i;
				else if (token_cmp(tkn, ">")) *ret = *ret > i;
				else if (token_cmp(tkn, ">=")) *ret = *ret >= i;
				else if (token_cmp(tkn, "==")) *ret = *ret == i;
				else if (token_cmp(tkn, "!=")) *ret = *ret != i;
				else if (token_cmp(tkn, "&")) *ret = *ret & i;
				else if (token_cmp(tkn, "^")) *ret = *ret ^ i;
				else if (token_cmp(tkn, "|")) *ret = *ret | i;
				else if (token_cmp(tkn, "&&")) *ret = *ret && i;
				else if (token_cmp(tkn, "||")) *ret = *ret || i;
				else if (token_cmp(tkn, ",")) *ret = i;
				else {
					fprintf(stderr, "Not implemented ");
					rh_dump_token(stdout, tkn);
					*ret = 1;
				}
			}
		}
	}
}/*}}}*/

// 本来式オブジェクトを返すべき
void rh_execute_expression(rh_context *ctx, int *ret, int enabled) {
	rh_execute_expression_internal(ctx, ret, 16, enabled);
}

void rh_execute_statement(rh_context *ctx, int enabled) {
	int i;
	if (token_cmp_skip(ctx, "print")) {
		if (ctx->token->type == TKN_STRING) {
			char *c = ctx->token->file_begin;
			for (c++; c < ctx->token->file_end - 1; c++)
				if (enabled) putchar(*c);
			if (enabled) printf("\n");
			ctx->token = ctx->token->next;
		} else {
			rh_execute_expression(ctx, &i, enabled);
			if (enabled) printf("%d\n", i);
		}
		error_with_token(ctx, ";", 0);
	} else if (token_cmp_skip(ctx, "if")) {
		error_with_token(ctx, "(", "if");
		rh_execute_expression(ctx, &i, enabled);
		error_with_token(ctx, ")", 0);
		rh_execute_statement(ctx, enabled && i);
		if (token_cmp_skip(ctx, "else")) {
			rh_execute_statement(ctx, enabled && !i);
		}
	} else if (token_cmp_skip(ctx, "{")) {
		while (ctx->token != NULL && !token_cmp(ctx->token, "}")) {
			rh_execute_statement(ctx, enabled);
		}
		error_with_token(ctx, "}", 0);
	} else if (token_cmp_skip(ctx, ";")) {
		/* do nothing */
	// } else if ((type = get_type(ctx))->kind != TK_NULL) {
	// 	statment->type = STAT_BLANK;
	// 	rh_type *tp = ctx->type_top;
	// 	while (tp) {
	// 		if (tp->level != level || strcmp(tp->token->text, type->token->text) == 0) break;
	// 		tp = tp->next;
	// 	}
	// 	if (tp && tp->level == level) {
	// 		rh_free(type);
	// 		E_ERROR(ctx, type->token, "The name '%s' already exists.", type->token->text);
	// 	} else {
	// 		type->next = ctx->type_top;
	// 		type->level = level;
	// 		ctx->type_top = type;
	// 	}
	// 	error_with_token(ctx, ";", 0);
	} else {
		rh_execute_expression(ctx, &i, enabled);
		error_with_token(ctx, ";", 0);
	}
}

int rh_execute(rh_context *ctx) {
	while (ctx->token) rh_execute_statement(ctx, 1);
	return 0;
}


/* vim: set foldmethod=marker : */
