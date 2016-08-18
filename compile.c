#include "common.h"


rh_asm_exp *create_exp_from_token(rh_context *ctx) {/*{{{*/
	char *c = ctx->token->text;
	long long val_ll = 0;
	long double val_ld = 0.0;
	int i, j, a;
	rh_asm_exp *exp;
	if (ctx->token->type == TKN_NUMERIC) {
		rh_asm_exp_literal *exp_literal = malloc(sizeof(rh_asm_exp_literal));
		exp = (rh_asm_exp *) exp_literal;
		exp_literal->children = 0;
		exp_literal->intval = 0;
		exp_literal->dblval = 0.0;
		exp_literal->type = TYPE_INT;
		int is_hexadecimal = 0;

		if (*c == '0') {
			c++;
			if (*c == 'x' || *c == 'X') {
				is_hexadecimal = 1;
				c++;
				for (;;) {
					if (ctbl[*c] & CP_10DIGIT) val_ll = *c - '0';
					else if (ctbl[*c] & CP_16DIGIT) 
						val_ll = *c - (ctbl[*c] & CP_CAPITAL ? 'A' : 'a') + 10;
					else break;
					exp_literal->intval = exp_literal->intval * 16 + val_ll; 
					c++;
				}
				exp_literal->dblval = (long double) exp_literal->intval;
			} else {	// octadecimal number or 0
				while (ctbl[*c] & CP_8DIGIT) {
					exp_literal->intval = exp_literal->intval * 8 + (*c - '0');
					c++;
				}
			}
		} else {
			while (ctbl[*c] & CP_10DIGIT) {
				exp_literal->intval = exp_literal->intval * 10 + (*c - '0');
				c++;
			}
		}
		if (*c == '.') {
			exp_literal->type = TYPE_DOUBLE;
			exp_literal->dblval = (long double) exp_literal->intval;
			val_ll = 10;
			c++;
			while (ctbl[*c] & CP_10DIGIT) {
				exp_literal->dblval += (*c - '0') / (double) val_ll;
				val_ll *= 10;
				c++;
			}
		}
		if (is_hexadecimal && (*c == 'P' || *c == 'p') || !is_hexadecimal && (*c == 'E' || *c == 'e')) {
			exp_literal->type = TYPE_DOUBLE;
			c++;
			val_ld = *c == '-' ? 0.1 : 1.0;
			if (*c == '-' || *c == '+') c++;	// token.cの問題よりまだ扱えない
			if (ctbl[*c] & CP_10DIGIT) {
				val_ll = 0;
				do val_ll = val_ll * 10 + (*c++ - '0'); while (ctbl[*c] & CP_10DIGIT);
				for (; val_ll; val_ll--) {
					exp_literal->intval *= val_ld;
					exp_literal->dblval *= val_ld;
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
		if (ctbl[*c] & CP_IDENT) {
			E_ERROR(ctx, ctx->token, "Missing in flag\n");
		}
	} else {
		E_FATAL(ctx, 0, "Invalid literal type\n");
	}
	return exp;

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
		{0, 0}
	};
	int i;
	for (i = 0; priority_table[i].symbol; i++) {
		if (strcmp(priority_table[i].symbol, token->text) == 0) {
			return priority_table[i].priority;
		}
	}
	return -1;
}/*}}}*/

void dump_expression(rh_asm_exp *exp) {
	if (exp->children == 0) {
		rh_asm_exp_literal *exp_literal = (rh_asm_exp_literal *) exp;
		printf("  %d  ", exp_literal->intval);
	} else {
		rh_asm_exp_terms *exp_terms = (rh_asm_exp_terms *) exp;
		printf(" < ");
		dump_expression(exp_terms->items[0]);
		printf(" , ");
		dump_expression(exp_terms->items[1]);
		printf(" > ");
	}
}

// ref: http://www.bohyoh.com/CandCPP/C/operator.html
rh_asm_exp *rh_compile_exp_internal(rh_context *ctx, int priority) {
	rh_asm_exp *exp, *exp1, *exp2;
	rh_asm_exp_terms *exp_terms;
	rh_token *token;
	int has_op = 0;
	if (priority == 0) {
		exp = create_exp_from_token(ctx);
		rh_next_token(ctx);
	} else {
		exp = rh_compile_exp_internal(ctx, priority - 1);
		while (get_priority(ctx->token) == priority) {
			token = ctx->token;
			rh_next_token(ctx);
			exp2 = rh_compile_exp_internal(ctx, priority - 1);
			exp1 = exp;
			exp_terms = malloc(sizeof(rh_asm_exp_terms));
			exp = (rh_asm_exp *) exp_terms;
			exp_terms->token = token;
			exp_terms->items[0] = exp1; exp_terms->items[1] = exp2;
			exp->children = 2;
		}
	}
	return exp;
}

rh_asm_exp *rh_compile_exp(rh_context *ctx) {
	return rh_compile_exp_internal(ctx, 16);
}
// TODO: rh_compile_func, rh_compile_statment

rh_asm_global rh_compile_global(rh_context *ctx) {
	rh_asm_global global;
	global.exp = rh_compile_exp(ctx);

	return global;
}

rh_asm_global rh_compile(rh_context *ctx) {
	return rh_compile_global(ctx);
}



/* vim: set foldmethod=marker : */
