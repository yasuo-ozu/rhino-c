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

rh_variable *search_declarator(rh_context *ctx, rh_token *token) {
	rh_variable *decl = ctx->var;
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

rh_variable *rh_execute_expression(rh_context *ctx, int enabled, int is_vector);

rh_variable *expression_with_paren(rh_context *ctx, int enabled) {
	rh_variable *ret;
	error_with_token(ctx, "(", 0);
	ret = rh_execute_expression(ctx, enabled, 0);
	error_with_token(ctx, ")", 0);
	return ret;
}

int is_equal_operator(rh_token *token) {
	return token->type == TKN_SYMBOL && *(token->file_end - 1) == '='
	   && *token->file_begin != '>' && *token->file_begin != '<';
}

rh_type *read_type_specifier(rh_context *ctx) {
	rh_type *ret = NULL;
	int is_s = 0, is_u = 0, long_count = 0, size = -2, count = 0;
	for(;;){
		if (token_cmp_skip(ctx, "unsigned")) is_u++, size = -1;
		else if (token_cmp_skip(ctx, "signed")) is_s++, size = -1;
		else if (token_cmp_skip(ctx, "long")) long_count++, size = -1;
		else if (token_cmp_skip(ctx, "char")) size = 1, count++;
		else if (token_cmp_skip(ctx, "short")) size = 2, count++;
		else if (token_cmp_skip(ctx, "int")) size = 4, count++;
		else break;
	}
	if (size > -2) {
		ret = rh_create_type(ctx);
		if (size == -1) size = 4;
		if (is_s + is_u > 1 || count > 1 || long_count > 2) {
			E_ERROR(ctx, 0, "Type error");
		}
		if (long_count) {
			if (size != 4) {
				E_ERROR(ctx, 0, "Type error");
				size = 4;
			}
			size *= long_count > 2 ? 2 : long_count;
		}
		ret->specifier = SP_NUMERIC;
		ret->size = size;
		ret->sign = is_u ? 0 : 1;
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
			int i = rh_variable_to_int(ctx, rh_execute_expression(ctx, 0, 1));
			error_with_token(ctx, "]", 0);
			ret2 = rh_create_type(ctx);
			ret2->length = i;
			ret2->child = ret;
			ret2->size = ret->size * i;
			ret = ret2;
		}
		if (ptr) {
			ret2 = rh_create_type(ctx);
			ret2->is_pointer = 1;
			ret2->specifier = SP_NULL;
			ret2->size = 4;
			ret = ret2;
		}
	}
	return ret;
}

/*
#define CALC_OP1_NUMERIC(out,op,in,size,sign) \
	(size==8?(sign?(*(long long*)out=op *(long long*)in):\
			  (*(unsigned long long*)out=op *(unsigned long long*)in)):\
	(sign?(*(unsigned*)out=op *(unsigned*)in):\
			  (*(int*)out=op *(int*)in)))
*/

rh_variable *rh_execute_calculation1(rh_context *ctx, rh_variable *var, rh_token *tkn) {
	unsigned char in[16], out[16];
	int j = 0;
	rh_type *type = rh_create_type(ctx);
	memset(in, 0, 16);
	memset(out, 0, 16);
	memcpy(in, var->memory, var->type->size);
	type->specifier = var->type->specifier;
	type->size = var->type->size;
	type->sign = var->type->sign;
	rh_variable *ret = rh_create_variable(ctx, type);
	if (token_cmp(tkn, "+")) *((long long *)out) = +*((long long *)in);
	if (token_cmp(tkn, "-")) *((long long *)out) = -*((long long *)in);
	if (token_cmp(tkn, "~")) *((long long *)out) = ~*((long long *)in);
	if (token_cmp(tkn, "!")) *((long long *)out) = !*((long long *)in);
	if ((j = token_cmp(tkn, "++")) || token_cmp(tkn, "--")) {
		if (var->is_left) {
			if (j) *((long long *)out) = ++(*((long long *)in));
			else *((long long *)out) = --(*((long long *)in));
			memcpy(var->memory, in, type->size);
		} else {
			E_ERROR(ctx, 0, "leftval err");
		}
	} else {
		fprintf(stderr, "Not implemented ");
		rh_dump_token(stdout, tkn);
	}
	memcpy(ret->memory, out, type->size);
	ret->next = ctx->var_t;
	ctx->var_t = ret;
	return ret;
}

rh_variable *rh_execute_calculation4(rh_context *ctx, rh_variable *var, rh_token *tkn) {
	unsigned char in[16], out[16];
	int j = 0;
	rh_type *type = rh_create_type(ctx);
	memset(in, 0, 16);
	memset(out, 0, 16);
	memcpy(in, var->memory, var->type->size);
	type->specifier = var->type->specifier;
	type->size = var->type->size;
	type->sign = var->type->sign;
	rh_variable *ret = rh_create_variable(ctx, type);
	if (var->is_left) {
		if (token_cmp(tkn, "++")) *((long long *)out) = ++*((long long *)in);
		if (token_cmp(tkn, "--")) *((long long *)out) = --*((long long *)in);
		memcpy(var->memory, in, type->size);
	} else {
		E_ERROR(ctx, 0, "lval err");
	}
	memcpy(ret->memory, out, type->size);
	ret->next = ctx->var_t;
	ctx->var_t = ret;
	return ret;
}

#define MAX(a,b)	((a)>(b)?(a):(b))
#define MIN(a,b)	((a)<(b)?(a):(b))

rh_variable *rh_execute_calculation2(rh_context *ctx, rh_variable *var1, rh_variable *var2, rh_token *tkn) {
	unsigned char in1[16], in2[16], out[16];
	int j = 0;
	rh_type *type = rh_create_type(ctx);
	memset(in1, 0, 16);
	memset(in2, 0, 16);
	memset(out, 0, 16);
	memcpy(in1, var1->memory, var1->type->size);
	memcpy(in2, var2->memory, var2->type->size);
	type->specifier = var1->type->specifier;
	type->size = MAX(var1->type->size, var2->type->size);
	type->sign = MIN(var1->type->sign, var2->type->sign);
	rh_variable *ret = rh_create_variable(ctx, type);
	if (var1->is_left) {
		if (token_cmp(tkn, "=")) *((long long *)out) = *((long long *)in1) = *((long long *)in2);
		else if (token_cmp(tkn, "+=")) *((long long *)out) = *((long long *)in1) += *((long long *)in2);
		else if (token_cmp(tkn, "-=")) *((long long *)out) = *((long long *)in1) -= *((long long *)in2);
		else if (token_cmp(tkn, "*=")) *((long long *)out) = *((long long *)in1) *= *((long long *)in2);
		else if (token_cmp(tkn, "/=")) *((long long *)out) = *((long long *)in1) /= *((long long *)in2);
		else if (token_cmp(tkn, "%=")) *((long long *)out) = *((long long *)in1) %= *((long long *)in2);
		else if (token_cmp(tkn, "<<=")) *((long long *)out) = *((long long *)in1) <<= *((long long *)in2);
		else if (token_cmp(tkn, ">>=")) *((long long *)out) = *((long long *)in1) >>= *((long long *)in2);
		else if (token_cmp(tkn, "^=")) *((long long *)out) = *((long long *)in1) ^= *((long long *)in2);
		else if (token_cmp(tkn, "!=")) *((long long *)out) = *((long long *)in1) != *((long long *)in2);
		else {
			E_ERROR(ctx, 0, "invalid equal operator");
		}
		memcpy(var1->memory, in1, var1->type->size);
	} else {
		E_ERROR(ctx, 0, "lval err");
	}
	memcpy(ret->memory, out, type->size);
	ret->next = ctx->var_t;
	ctx->var_t = ret;
	return ret;
}

rh_variable *rh_execute_calculation3(rh_context *ctx, rh_variable *var1, rh_variable *var2, rh_token *tkn) {
	unsigned char in1[16], in2[16], out[16];
	int j = 0;
	rh_type *type = rh_create_type(ctx);
	memset(in1, 0, 16);
	memset(in2, 0, 16);
	memset(out, 0, 16);
	memcpy(in1, var1->memory, var1->type->size);
	memcpy(in2, var2->memory, var2->type->size);
	type->specifier = var1->type->specifier;
	type->size = MAX(var1->type->size, var2->type->size);
	type->sign = MIN(var1->type->sign, var2->type->sign);
	rh_variable *ret = rh_create_variable(ctx, type);
	if (token_cmp(tkn, "+")) *((long long *)out) = *((long long *)in1) + *((long long *)in2);
	else if (token_cmp(tkn, "-")) *((long long *)out) = *((long long *)in1) - *((long long *)in2);
	else if (token_cmp(tkn, "*")) *((long long *)out) = *((long long *)in1) * *((long long *)in2);
	else if (token_cmp(tkn, "/")) *((long long *)out) = *((long long *)in1) / *((long long *)in2);
	else if (token_cmp(tkn, "%")) *((long long *)out) = *((long long *)in1) % *((long long *)in2);
	else if (token_cmp(tkn, "<<")) *((long long *)out) = *((long long *)in1) << *((long long *)in2);
	else if (token_cmp(tkn, ">>")) *((long long *)out) = *((long long *)in1) >> *((long long *)in2);
	else if (token_cmp(tkn, "<")) *((long long *)out) = *((long long *)in1) < *((long long *)in2);
	else if (token_cmp(tkn, "<=")) *((long long *)out) = *((long long *)in1) <= *((long long *)in2);
	else if (token_cmp(tkn, ">")) *((long long *)out) = *((long long *)in1) > *((long long *)in2);
	else if (token_cmp(tkn, ">=")) *((long long *)out) = *((long long *)in1) >= *((long long *)in2);
	else if (token_cmp(tkn, "==")) *((long long *)out) = *((long long *)in1) == *((long long *)in2);
	else if (token_cmp(tkn, "!=")) *((long long *)out) = *((long long *)in1) != *((long long *)in2);
	else if (token_cmp(tkn, "&")) *((long long *)out) = *((long long *)in1) & *((long long *)in2);
	else if (token_cmp(tkn, "^")) *((long long *)out) = *((long long *)in1) ^ *((long long *)in2);
	else if (token_cmp(tkn, "|")) *((long long *)out) = *((long long *)in1) | *((long long *)in2);
	else if (token_cmp(tkn, "&&")) *((long long *)out) = *((long long *)in1) && *((long long *)in2);
	else if (token_cmp(tkn, "||")) *((long long *)out) = *((long long *)in1) || *((long long *)in2);
	else if (token_cmp(tkn, ",")) *((long long *)out) = *((long long *)in1) , *((long long *)in2);
	else {
		E_ERROR(ctx, 0, "invalid equal operator");
	}
	memcpy(ret->memory, out, type->size);
	ret->next = ctx->var_t;
	ctx->var_t = ret;
	return ret;
}

/* When enabled==0, supress side effects. */
rh_variable *rh_execute_expression_internal(rh_context *ctx, int priority, int enabled, int is_vector) {/*{{{*/
	int has_op = 0, i, j;
	rh_variable *ret = NULL;
	rh_token *tkn, *tkn0;
	if (priority == 0) {
		if (ctx->token->type == TKN_NUMERIC) {
			ret = ctx->token->var;
			ret->next = ctx->var_t;
			ctx->var_t = ret;
			ctx->token = ctx->token->next;
		} else if (token_cmp(ctx->token, "("))
			ret = expression_with_paren(ctx, enabled);
		else if (ctx->token->type == TKN_IDENT) {
			rh_variable *var = search_declarator(ctx, ctx->token);
			if (var) ret = var;
			else {
				E_ERROR(ctx, ctx->token, "declarator not defined");
			}
			ctx->token = ctx->token->next;
		} else {
			E_ERROR(ctx, 0, "Invalid endterm");
			ctx->token = ctx->token->next;
		}
	} else {
		if (get_priority(ctx->token, 2) == priority) {
			tkn = ctx->token;
			tkn0 = ctx->token = ctx->token->next;
			ret = rh_execute_expression_internal(ctx, priority, enabled, 0);
			if (enabled) ret = rh_execute_calculation1(ctx, ret, tkn);
		} else {
			tkn0 = ctx->token;
			ret = rh_execute_expression_internal(ctx, priority - 1, enabled, 0);
			if (is_vector && token_cmp(ctx->token, ",")) return;
			while (get_priority(ctx->token, 1) == priority) {
				if (is_equal_operator(ctx->token)) {
					rh_token *eq_buf[20], *eq_exp[20], *etop;
					int bc = 0, ec = 0;
					eq_exp[ec++] = tkn0;
					do {
						eq_buf[bc++] = ctx->token;
						eq_exp[ec++] = ctx->token = ctx->token->next;
						rh_execute_expression_internal(ctx, priority - 1, 0, 0);
					} while (is_equal_operator(ctx->token));
					etop = ctx->token;
					if (!enabled) break;
					ctx->token = eq_exp[--ec];
					ret = rh_execute_expression_internal(ctx, priority - 1, 1, 0);
					while (bc) {
						ctx->token = eq_exp[--ec];
						rh_variable *var = rh_execute_expression_internal(ctx, priority - 1, 1, 0);
						ret = rh_execute_calculation2(ctx, var, ret, eq_buf[--bc]);
					}
					ctx->token = etop;
					break;
				} else {
					tkn = ctx->token;
					ctx->token = ctx->token->next;
					rh_variable *ret2 = rh_execute_expression_internal(ctx, priority - 1, enabled, 0);
					if (enabled) ret = rh_execute_calculation2(ctx, ret, ret2, tkn);
				}
			}
			while (get_priority(ctx->token, 4) == priority) {
				tkn = ctx->token;
				if (token_cmp_skip(ctx, "++") || token_cmp_skip(ctx, "--")) {
					if (enabled) ret = rh_execute_calculation4(ctx, ret, tkn);
				} else {
					fprintf(stderr, "Not implemented ");
					rh_dump_token(stdout, tkn);
				}
			}
		}
	}
	return ret;
}/*}}}*/

// 本来式オブジェクトを返すべき
rh_variable *rh_execute_expression(rh_context *ctx, int enabled, int is_vector) {
	return rh_execute_expression_internal(ctx, 16, enabled, is_vector);
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
			rh_variable *var = rh_execute_expression(ctx, enabled, 0);
			if (enabled) printf("%d\n", rh_variable_to_int(ctx, var));
		}
	} else if (token_cmp_skip(ctx, "input")) {
		rh_variable *var = rh_execute_expression(ctx, enabled, 0);
		if (var->is_left)
		   if (enabled)	scanf("%d", ((int *)var->memory));
		else {
			E_ERROR(ctx, ctx->token, "declarator not defined");
		}
		ctx->token = ctx->token->next;
	} else if (token_cmp_skip(ctx, "random")) {
		rh_variable *var = rh_execute_expression(ctx, enabled, 0);
		static int rand_before = -1;
		if (!~rand_before) rand_before = 1, srand(time(NULL));
		if (var->is_left)
		   if (enabled) *((int *)var->memory) = rand() / (RAND_MAX + 1.0) * 256;
		else {
			E_ERROR(ctx, ctx->token, "declarator not defined");
		}
		ctx->token = ctx->token->next;
	} else if (token_cmp_skip(ctx, "if")) {
		rh_variable *var = expression_with_paren(ctx, enabled);
		rh_variable_to_int(ctx, var);
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
		while (ctx->var != NULL && ctx->var->depth >= ctx->depth) {
			rh_variable *var = ctx->var;
			//ctx->sp += var->type->size;
			ctx->var = var->next;
			rh_free(var);
		}
		ctx->depth--;
		error_with_token(ctx, "}", 0);
		needs_semicolon = 0;
	} else if (token_cmp_skip(ctx, "while")) {
		rh_token *tkn = ctx->token;
		for (;;) {
			i = rh_variable_to_int(ctx, expression_with_paren(ctx, enabled));
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
			i = rh_variable_to_int(ctx, expression_with_paren(ctx, enabled));
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
		i = rh_variable_to_int(ctx, rh_execute_expression(ctx, enabled, 0));
		error_with_token(ctx, ";", 0);
		rh_token *tkn = ctx->token, *tkn0;
		for (;;) {
			i = rh_variable_to_int(ctx, rh_execute_expression(ctx, enabled, 0));
			error_with_token(ctx, ";", 0);
			tkn0 = ctx->token;
			rh_execute_expression(ctx, 0, 0);
			error_with_token(ctx, ")", 0);
			res = rh_execute_statement(ctx, enabled && i);
			if (!enabled || !i || res == SR_BREAK) {
					res = SR_NORMAL; break;
			} else if (res == SR_RETURN) break;
			ctx->token = tkn0;
			rh_execute_expression(ctx, enabled, 0);
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
				rh_variable *var = rh_create_variable(ctx, NULL), *var2;
				typ2 = read_type_declarator(ctx, typ, &var->token, 1);
				var->memory = rh_malloc(typ2->size);
				var->token = ctx->token;
				var->depth = ctx->depth;
				//ctx->sp -= declarator->size;
				var->next = ctx->var;
				//var->memory = ctx->sp;
				var->is_left = 1;
				ctx->var = var;
				if (token_cmp_skip(ctx, "=")) {
					var2 = rh_execute_expression(ctx, enabled, 1);
					if (enabled) {
						rh_assign(ctx, var, var2);
					}
				}
			} while(token_cmp_skip(ctx, ","));
		} else {
			rh_execute_expression(ctx, enabled, 0);
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
