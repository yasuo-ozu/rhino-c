#include "common.h"

// ref: http://www.bohyoh.com/CandCPP/C/operator.html
rh_asm_exp *rh_compile_exp_internal(rh_context *ctx, int priority) {
	rh_asm_exp *exp, *exp1, *exp2;
	rh_token token;
	int has_op = 0;
	if (priority == 0) {
		if (ctx->compile.token.kind == TK_VAL) {
			exp = malloc(sizeof(rh_asm_exp));
			exp->token = ctx->compile.token;
			ctx->compile.token = rh_next_token(ctx);
			return exp;
		} else {
			E_FATAL(ctx, &ctx->compile.token, "This token is not end symbol.\n");
			exit(1);
		}
	} else {
		exp1 = rh_compile_exp_internal(ctx, priority - 1);
		if (priority == 4 && (ctx->compile.token.kind == TK_MUL || ctx->compile.token.kind == TK_DIV) ||
			priority == 5 && (ctx->compile.token.kind == TK_PLUS || ctx->compile.token.kind == TK_MINUS)) {
			token = ctx->compile.token;
			ctx->compile.token = rh_next_token(ctx);
			exp2 = rh_compile_exp_internal(ctx, priority - 1);	// priority ?? 左再帰性
			exp = malloc(sizeof(rh_asm_exp));
			exp->token = token;
			exp->arg1 = exp1; exp->arg2 = exp2;
			return exp;
		}
		return exp1;
	}
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
