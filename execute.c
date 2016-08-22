#include "common.h"

int token_cmp(rh_token *token, char *ident) {/*{{{*/
	char *c = token->file_begin;
	while (*ident) {
		if (*c != *ident) return 0;
		c++; ident++;
	}
	return 1;
}/*}}}*/

int get_priority(rh_token *token) {/*{{{*/
	struct {
		char *symbol; int priority;
	} priority_table[] = {
		{"*", 4},
		{"/", 4},
		{"%", 4},
		{"+", 5},
		{"-", 5},
		{"=", 15},
		{0, 0}
	};
	int i;
	for (i = 0; priority_table[i].symbol; i++) {
		if (token_cmp(token, priority_table[i].symbol)) {
			return priority_table[i].priority;
		}
	}
	return -1;
}/*}}}*/

rh_token *error_with_token(rh_context *ctx, rh_token *token, char *require, char *after) {/*{{{*/
	if (token_cmp(token, require)) {
		if (after) {
			E_ERROR(ctx, token, "requires '%s' after '%s'", require, after);
		} else {
			E_ERROR(ctx, token, "requires '%s'", require);
		}
		return token;
	} else {
		return token->next;
	}
}/*}}}*/

rh_token *rh_execute_expression_internal(rh_context *ctx, rh_token *token, int *ret, int priority) {/*{{{*/
	int has_op = 0, i, j;
	rh_token *tkn;
	if (priority == 0) {
		if (token->type == TKN_NUMERIC) {
			*ret = token->literal.intval;
		} else {
			E_ERROR(ctx, 0, "Invalid endterm");
			*ret = 1;
		}
		token = token->next;
	} else {
		rh_execute_expression_internal(ctx, token, ret, priority - 1);
		while (get_priority(token) == priority) {
			tkn = ctx->token;
			token = token->next;
			token = rh_execute_expression_internal(ctx, token, &i, priority - 1);
			if (token_cmp(tkn, "+")) *ret += i;
			if (token_cmp(tkn, "-")) *ret -= i;
			if (token_cmp(tkn, "*")) *ret *= i;
			if (token_cmp(tkn, "/")) *ret /= i;
			if (token_cmp(tkn, "%")) *ret %= i;
			else {
				fprintf(stderr, "Not implemented ");
				rh_dump_token(stdout, tkn);
				*ret = 1;
			}
		}
	}
	return token;
}/*}}}*/

// 本来式オブジェクトを返すべき
rh_token *rh_execute_expression(rh_context *ctx, rh_token *token, int *ret) {
	return rh_execute_expression_internal(ctx, token, ret, 16);
}

rh_token *rh_execute_statement(rh_context *ctx, rh_token *token) {
	int i;
	if (token_cmp(token, "if")) {
		token = token->next;
		token = error_with_token(ctx, token, "(", "if");
		token = rh_execute_expression(ctx, token, &i);
		token = error_with_token(ctx, token, ")", 0);
		token = rh_execute_statement(ctx, token);
		if (token_cmp(token, "else")) {
			token = token->next;
			token = rh_execute_statement(ctx, token);
		}
	} else if (token_cmp(token, "{")) {
		token = token->next;
		while (token->type != TKN_NULL && token_cmp(token, "}")) {
			rh_execute_statement(ctx, token);
		}
		token = error_with_token(ctx, token, "}", 0);
	} else if (token_cmp(token, ";")) {
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
		token = rh_execute_expression(ctx, token, &i);
		token = error_with_token(ctx, token, ";", 0);
		printf("# %d\n", i);
	}
}

int rh_execute(rh_context *ctx, rh_token *token) {
	while (token != NULL) token = rh_execute_statement(ctx, token);
	return 0;
}


/* vim: set foldmethod=marker : */
