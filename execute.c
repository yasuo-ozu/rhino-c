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
		{"&&", 13, 1}, {"&&", 14, 1}, {"=", 15, 1}, {"+=", 15, 1}, {"-=", 15, 1}, 
		{"*=", 15, 1}, {"/=", 15, 1}, {"%=", 15, 1}, {"<<=", 15, 1}, {">>=", 15, 1}, 
		{"&=", 15, 1}, {"^=", 15, 1}, {"|=", 15, 1}, {",", 16, 1}, {0, 0, 1}
	};
	int i;
	for (i = 0; priority_table[i].symbol; i++) {
		if (priority_table[i].type & type && token_cmp(token, priority_table[i].symbol)) {
			return priority_table[i].priority;
		}
	}
	return -1;
}/*}}}*/

rh_declarator *search_declarator(rh_context *ctx, rh_token *token) {
	rh_declarator *decl = ctx->declarator;
	if (token->type != TKN_IDENT) return NULL;
	while (decl) {
		char *a = token->file_begin,
		     *b = decl->token->file_begin;
		while (a < token->file_end &&
			b < decl->token->file_end) {
			if (*a != *b) break;
			a++; b++;
		}
		if (a == token->file_end &&
			b == decl->token->file_end) break;
		decl = decl->next;
	}
	return decl;
}

void error_with_token(rh_context *ctx, char *require, char *after) {/*{{{*/
	if (!token_cmp_skip(ctx, require)) {
		if (after) {
			E_ERROR(ctx, ctx->token, "requires '%s' after '%s'", require, after);
		} else {
			E_ERROR(ctx, ctx->token, "requires '%s'", require);
		}
	}
}/*}}}*/

void rh_execute_expression(rh_context *ctx, int *ret, int enabled, int is_vector);

void expression_with_paren(rh_context *ctx, int *ret, int enabled) {
	error_with_token(ctx, "(", 0);
	rh_execute_expression(ctx, ret, enabled, 0);
	error_with_token(ctx, ")", 0);
}

int is_equal_operator(rh_token *token) {
	return token->type == TKN_SYMBOL && *(token->file_end - 1) == '='
	   && *token->file_begin != '>' && *token->file_begin != '<';
}

rh_type *read_type_specifier(rh_context *ctx) {
	rh_type *ret = NULL;
	int is_s = 0, is_u = 0, long_count = 0, size = -2;
	for(;;){
		if (token_cmp_skip(ctx, "unsigned")) is_u = 1, size = -1;
		else if (token_cmp_skip(ctx, "signed")) is_s = 1, size = -1;
		else if (token_cmp_skip(ctx, "long")) long_count++, size = -1;
		else if (token_cmp_skip(ctx, "char")) size = 1;
		else if (token_cmp_skip(ctx, "short")) size = 2;
		else if (token_cmp_skip(ctx, "int")) size = 4;
		else break;
	}
	if (size > -2) {
		ret = rh_malloc(sizeof(rh_type));
		if (size == -1) size = 4;
		if (is_s && is_u) {
			E_ERROR(ctx, 0, "Type error");
		}
		if (long_count) {
			if (size != 4) {
				E_ERROR(ctx, 0, "Type error");
				size = 4;
			}
			if (long_count > 2) {
				E_ERROR(ctx, 0, "Type error");
				long_count = 2;
			}
			size *= long_count;
		}
		ret->specifier = SP_NUMERIC;
		ret->size = size;
		ret->sign = is_u ? 0 : 1;
		ret->child = NULL;
		ret->is_pointer = 0;
		ret->length = 0;
	}
	return ret;
}

/* require_ident 1: require(variable declaration), -1: no(casting, ...), 0: any(function prototype arguments) */
rh_type *read_type_declarator(rh_context *ctx, rh_type *parent, rh_token **p_token, int require_ident) {
	rh_type *ret;
	if (ctx->token->type == TKN_IDENT) {
		if (require_ident == -1) {
			E_ERROR(ctx, ctx->token, "Invalid declarator");
		}
		if (p_token) *p_token = ctx->token;
		ret = parent;
	} else if (token_cmp_skip(ctx, "(")) {
		ret = read_type_declarator(ctx, parent, p_token, require_ident);
		error_with_token(ctx, ")", 0);
	} else {
		int ptr = 0;
		rh_type *ret2;
		if (token_cmp_skip(ctx, "*")) ptr = 1;
		ret = read_type_declarator(ctx, parent, p_token, require_ident);
		if (token_cmp_skip(ctx, "[")) {
			int i;
			rh_execute_expression(ctx, &i, 0, 1);
			error_with_token(ctx, "]", 0);
			ret2 = rh_malloc(sizeof(rh_type));
			ret2->length = i;
			ret2->is_pointer = 0;
			ret2->child = ret;
			ret2->specifier = SP_NULL;
			ret2->size = ret->sign = 0;
			ret = ret2;
		}
		if (ptr) {
			ret2 = rh_malloc(sizeof(rh_type));
			ret2->length = 0;
			ret2->is_pointer = 1;
			ret2->child = ret;
			ret2->specifier = SP_NULL;
			ret2->size = ret->sign = 0;
			ret = ret2;
		}
	}
}

/* When enabled==0, supress side effects. */
rh_declarator *rh_execute_expression_internal(rh_context *ctx, int priority, int enabled, int is_vector) {/*{{{*/
	int has_op = 0, i, j;
	rh_declarator *ret;
	rh_token *tkn, *tkn0;
	ret = rh_alloc(sizeof(rh_declarator));
	ret->token = NULL;
	ret->next = NULL;
	ret->depth = -1;
	ret->type = rh_alloc(sizeof(rh_type));
	ret->type->size = 4; ret->type->sign = 1; ret->type->specifier = SP_NUMERIC; ret->type->length = 0;
	ret->type->is_pointer = 0; ret->type->child = NULL;
	ret->memory = rh_malloc(4);
	*((int *)ret->memory) = 1;
	if (priority == 0) {
		if (ctx->token->type == TKN_NUMERIC) {
			*ret = ctx->token->literal.intval;
			ctx->token = ctx->token->next;
		} else if (token_cmp(ctx->token, "(")) {
			expression_with_paren(ctx, ret, enabled);
		} else if (ctx->token->type == TKN_IDENT) {
			rh_declarator *decl = search_declarator(ctx, ctx->token);
			if (decl) {
				rh_free(ret->type);
				rh_free(ret);
				ret = decl;
			} else {
				E_ERROR(ctx, ctx->token, "declarator not defined");
			}
			ctx->token = ctx->token->next;
		} else {
			E_ERROR(ctx, 0, "Invalid endterm");
			ctx->token = ctx->token->next;
			*ret = 1;
		}
	} else {
		if (get_priority(ctx->token, 2) == priority) {
			tkn = ctx->token;
			tkn0 = ctx->token = ctx->token->next;
			rh_execute_expression_internal(ctx, ret, priority, enabled, 0);
			if (token_cmp(tkn, "+")) *ret = *ret;
			else if (token_cmp(tkn, "-")) *ret = -*ret;
			else if (token_cmp(tkn, "!")) *ret = !*ret;
			else if (token_cmp(tkn, "~")) *ret = ~*ret;
			else if ((j = token_cmp(tkn, "++")) || token_cmp(tkn, "--")) {
				rh_declarator *decl = search_declarator(ctx, tkn0);
				if (decl) {
					if (enabled) {
						if (j) *ret = ++*((int *)decl->memory);
						else *ret = --*((int *)decl->memory);
					}
				} else {
					E_ERROR(ctx, ctx->token, "declarator not defined");
					*ret = 1;
				}
			} else {
				fprintf(stderr, "Not implemented ");
				rh_dump_token(stdout, tkn);
			}
		} else {
			tkn0 = ctx->token;
			rh_execute_expression_internal(ctx, ret, priority - 1, enabled, 0);
			if (is_vector && token_cmp(ctx->token, ",")) return;
			while (get_priority(ctx->token, 1) == priority) {
				if (is_equal_operator(ctx->token)) {
					rh_token *eq_buf[20], *eq_exp[20], *etop;
					int bc = 0, ec = 0;
					eq_exp[ec++] = tkn0;
					do {
						eq_buf[bc++] = ctx->token;
						eq_exp[ec++] = ctx->token = ctx->token->next;
						rh_execute_expression_internal(ctx, ret, priority - 1, 0, 0);
					} while (is_equal_operator(ctx->token));
					etop = ctx->token;
					if (!enabled) break;
					ctx->token = eq_exp[--ec];
					rh_execute_expression_internal(ctx, ret, priority - 1, 1, 0);
					while (bc) {
						rh_declarator *decl = search_declarator(ctx, eq_exp[--ec]);
						if (decl) {
							bc--;
							if (token_cmp(eq_buf[bc], "=")) *ret = *((int *)decl->memory) = *ret;
							else if (token_cmp(eq_buf[bc], "+=")) *ret = *((int *)decl->memory) += *ret;
							else if (token_cmp(eq_buf[bc], "-=")) *ret = *((int *)decl->memory) -= *ret;
							else if (token_cmp(eq_buf[bc], "*=")) *ret = *((int *)decl->memory) *= *ret;
							else if (token_cmp(eq_buf[bc], "/=")) *ret = *((int *)decl->memory) /= *ret;
							else if (token_cmp(eq_buf[bc], "%=")) *ret = *((int *)decl->memory) %= *ret;
							else if (token_cmp(eq_buf[bc], "<<=")) *ret = *((int *)decl->memory) <<= *ret;
							else if (token_cmp(eq_buf[bc], ">>=")) *ret = *((int *)decl->memory) >>= *ret;
							else if (token_cmp(eq_buf[bc], "&=")) *ret = *((int *)decl->memory) &= *ret;
							else if (token_cmp(eq_buf[bc], "^=")) *ret = *((int *)decl->memory) ^= *ret;
							else if (token_cmp(eq_buf[bc], "|=")) *ret = *((int *)decl->memory) != *ret;
							else {
								E_ERROR(ctx, eq_buf[bc], "invalid equal operator");
								*ret = 1; break;
							}
						} else {
							E_ERROR(ctx, ctx->token, "declarator not defined");
							*ret = 1; break;
						}
					}
					ctx->token = etop;
					break;
				} else {
					tkn = ctx->token;
					ctx->token = ctx->token->next;
					rh_execute_expression_internal(ctx, &i, priority - 1, enabled, 0);
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
					else if (token_cmp(tkn, "&")) *ret &= i;
					else if (token_cmp(tkn, "^")) *ret ^= i;
					else if (token_cmp(tkn, "|")) *ret |= i;
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
			while (get_priority(ctx->token, 4) == priority) {
				if ((j = token_cmp_skip(ctx, "++")) || token_cmp_skip(ctx, "--")) {
					rh_declarator *decl = search_declarator(ctx, tkn0);
					if (decl) {
						if (enabled) {
							if (j) *ret = (*((int *)decl->memory))++;
							else *ret = (*((int *)decl->memory))--;
						}
					} else {
						E_ERROR(ctx, ctx->token, "declarator not defined");
						*ret = 1;
					}
				} else {
					fprintf(stderr, "Not implemented ");
					rh_dump_token(stdout, tkn);
					*ret = 1;
				}
			}
		}
	}
	return ret;
}/*}}}*/

// 本来式オブジェクトを返すべき
void rh_execute_expression(rh_context *ctx, int *ret, int enabled, int is_vector) {
	rh_execute_expression_internal(ctx, ret, 16, enabled, is_vector);
}

typedef enum {/*{{{*/
	SR_NORMAL = 0, SR_RETURN, SR_CONTINUE, SR_BREAK
} rh_statement_result;

rh_statement_result rh_execute_statement(rh_context *ctx, int enabled) {
	int i, j, needs_semicolon = 1;
	rh_statement_result res = SR_NORMAL;
	if (token_cmp_skip(ctx, "time")) {
		clock_t t0 = clock();
		res = rh_execute_statement(ctx, enabled);
		if (enabled) printf("** time = %d[ms]\n", (int) (clock() - t0) / 1000);
		needs_semicolon = 0;
	} else if (token_cmp_skip(ctx, "print")) {
		if (ctx->token->type == TKN_STRING) {
			char *c = ctx->token->file_begin;
			for (c++; c < ctx->token->file_end - 1; c++) if (enabled) putchar(*c);
			if (enabled) printf("\n");
			ctx->token = ctx->token->next;
		} else {
			rh_execute_expression(ctx, &i, enabled, 0);
			if (enabled) printf("%d\n", i);
		}
	} else if (token_cmp_skip(ctx, "input")) {
		rh_declarator *decl = search_declarator(ctx, ctx->token);
		if (decl) scanf("%d", ((int *)decl->memory));
		else {
			E_ERROR(ctx, ctx->token, "declarator not defined");
		}
		ctx->token = ctx->token->next;
	} else if (token_cmp_skip(ctx, "random")) {
		rh_declarator *decl = search_declarator(ctx, ctx->token);
		static int rand_before = -1;
		if (!~rand_before) rand_before = 1, srand(time(NULL));
		if (decl) *((int *)decl->memory) = rand() / (RAND_MAX + 1.0) * 256;
		else {
			E_ERROR(ctx, ctx->token, "declarator not defined");
		}
		ctx->token = ctx->token->next;
	} else if (token_cmp_skip(ctx, "if")) {
		expression_with_paren(ctx, &i, enabled);
		res = rh_execute_statement(ctx, enabled && i);
		if (token_cmp_skip(ctx, "else"))
			res = rh_execute_statement(ctx, enabled && !i && res == SR_NORMAL);
		needs_semicolon = 0;
	} else if (token_cmp_skip(ctx, "{")) {
		rh_statement_result res2;
		ctx->depth++;
		while (ctx->token != NULL && !token_cmp(ctx->token, "}")) {
			res2 = rh_execute_statement(ctx, enabled);
			if (enabled && res2 != SR_NORMAL) res = res2, enabled = 0;
		}
		while (ctx->declarator != NULL && ctx->declarator->depth >= ctx->depth) {
			rh_declarator *decl = ctx->declarator;
			ctx->declarator = decl->next;
			rh_free(decl);
		}
		ctx->depth--;
		error_with_token(ctx, "}", 0);
		needs_semicolon = 0;
	} else if (token_cmp_skip(ctx, "while")) {
		rh_token *tkn = ctx->token;
		for (;;) {
			expression_with_paren(ctx, &i, enabled);
			res = rh_execute_statement(ctx, enabled && i);
			if (!enabled || !i || res == SR_BREAK) {
					res = SR_NORMAL;
					break;
			} else if (res == SR_RETURN) break;
			ctx->token = tkn;
		}
		needs_semicolon = 0;
	} else if (token_cmp_skip(ctx, "do")) {
		rh_token *tkn = ctx->token;
		for (;;) {
			res = rh_execute_statement(ctx, enabled);
			error_with_token(ctx, "while", 0);
			expression_with_paren(ctx, &i, enabled);
			error_with_token(ctx, ";", 0);
			if (!enabled || !i || res == SR_BREAK) {
					res = SR_NORMAL;
				   	break;
			} else if (res == SR_RETURN) break;
			ctx->token = tkn;
		}
		needs_semicolon = 0;
	} else if (token_cmp_skip(ctx, "for")) {
		error_with_token(ctx, "(", "for");
		rh_execute_expression(ctx, &i, enabled, 0);
		error_with_token(ctx, ";", 0);
		rh_token *tkn = ctx->token, *tkn0;
		for (;;) {
			rh_execute_expression(ctx, &i, enabled, 0);
			error_with_token(ctx, ";", 0);
			tkn0 = ctx->token;
			rh_execute_expression(ctx, &j, 0, 0);
			error_with_token(ctx, ")", 0);
			res = rh_execute_statement(ctx, enabled && i);
			if (!enabled || !i || res == SR_BREAK) {
					res = SR_NORMAL; break;
			} else if (res == SR_RETURN) break;
			ctx->token = tkn0;
			rh_execute_expression(ctx, &j, enabled, 0);
			ctx->token = tkn;
		}
		needs_semicolon = 0;
	} else if (token_cmp(ctx->token, ";")); /* do nothing */
	else if (token_cmp_skip(ctx, "break")) 	res = SR_BREAK;
	else if (token_cmp_skip(ctx, "continue")) 	res = SR_CONTINUE;
	else if (token_cmp_skip(ctx, "return")) 	res = SR_RETURN;
	else {
		rh_type *typ = read_type_specifier(ctx), *typ2;
		if (typ) {
			do {
				typ2 = read_type_declarator(ctx, typ, &declarator->token, 1);
				rh_declarator *declarator = rh_malloc(sizeof(rh_declarator)), *decl;
				declarator->type = typ2;
				declarator->token = ctx->token;
				declarator->depth = ctx->depth;
				declarator->size = rh_get_type_size(typ2);
				ctx->sp -= declarator->size;
				declarator->next = ctx->declarator;
				declarator->memory = ctx->sp;
				ctx->declarator = declarator;
				if (token_cmp_skip(ctx, "=")) {
					decl = rh_execute_expression(ctx, enabled, 1);
					if (enabled) {
						

					}
				}
			} while(token_cmp_skip(ctx, ","));
		} else {
			rh_execute_expression(ctx, &i, enabled, 0);
		}
	}
/*
	else if (token_cmp_skip(ctx, "int")) {
		do {
			if (ctx->token->type == TKN_IDENT) {
				// TODO: 既に登録された名前でないか確認
				// TODO: 識別子が予約語でないか確認
			} else {
				E_ERROR(ctx, ctx->token, "Identifier error");
			}
		} while(token_cmp_skip(ctx, ","));
		*/
	if (needs_semicolon) error_with_token(ctx, ";", 0);
	return res;
}/*}}}*/

int rh_execute(rh_context *ctx) {
	ctx->depth = 0;
	while (ctx->token) rh_execute_statement(ctx, 1);
	return 0;
}


/* vim: set foldmethod=marker : */
