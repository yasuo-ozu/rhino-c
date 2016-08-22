#include "common.h"


// rh_asm_exp *create_exp_from_token(rh_context *ctx) {/*{{{*/
// 	char *c = ctx->token->text;
// 	long long val_ll = 0;
// 	long double val_ld = 0.0;
// 	int i, j, a;
// 	rh_asm_exp *exp = rh_malloc(sizeof(rh_asm_exp));
// 	if (ctx->token->type == TKN_NUMERIC) {
// 	} else {
// 		rh_type *tp = ctx->type_top;
// 		while (tp) {
// 			if (strcmp(ctx->token->text, tp->token->text) == 0) {
// 				exp->type = EXP_VARIABLE;
// 				exp->var = tp;
// 				break;
// 			}
// 			tp = tp->next;
// 		}
// 		if (!tp) {
// 			E_FATAL(ctx, ctx->token, "Invalid identifier\n");
// 		}
// 	}
// 	return exp;
// 
// }/*}}}*/
// 
// // rh_type *get_type(rh_context *ctx) {/*{{{*/
// // 	rh_type *type = rh_malloc(sizeof(rh_type));
// // 	type->kind = TK_NULL;
// // 	type->token = NULL;
// // 	if (strcmp(ctx->token->text, "int") == 0) {
// // 		type->kind = TK_INT;
// // 		rh_next_token(ctx);
// // 	}
// // 	if (type->kind != TK_NULL && ctx->token->type == TKN_IDENT) {
// // 		type->token = ctx->token;
// // 		rh_next_token(ctx);
// // 	}
// // 	return type;
// // }/*}}}*/
// 
// void dump_expression(rh_asm_exp *exp) {/*{{{*/
// 	if (exp->type == EXP_LITERAL) {
// 		printf("  %d  ", exp->literal.intval);
// 	} else {
// 		printf(" < ");
// 		dump_expression(exp->op.exp[0]);
// 		printf(" , ");
// 		dump_expression(exp->op.exp[1]);
// 		printf(" > ");
// 	}
// }/*}}}*/
// 
// // ref: http://www.bohyoh.com/CandCPP/C/operator.html
// rh_asm_exp *rh_compile_exp_internal(rh_context *ctx, int priority) {
// 	rh_asm_exp *exp, *exp1, *exp2;
// 	rh_token *token;
// 	int has_op = 0;
// 	if (priority == 0) {
// 		exp = create_exp_from_token(ctx);
// 		rh_next_token(ctx);
// 	} else {
// 		exp = rh_compile_exp_internal(ctx, priority - 1);
// 		while (get_priority(ctx->token) == priority) {
// 			token = ctx->token;
// 			rh_next_token(ctx);
// 			exp2 = rh_compile_exp_internal(ctx, priority - 1);
// 			exp1 = exp;
// 			exp = malloc(sizeof(rh_asm_exp));
// 			exp->op.token = token;
// 			exp->op.exp[0] = exp1;
// 			exp->op.exp[1] = exp2;
// 			exp->type = EXP_BINARYOP;
// 		}
// 	}
// 	return exp;
// }
// 
// rh_asm_exp *rh_compile_exp(rh_context *ctx) {
// 	return rh_compile_exp_internal(ctx, 16);
// }
// // TODO: rh_compile_func, rh_compile_statment
// 
// void error_with_token(rh_context *ctx, char *require, char *after) {
// 	if (strcmp(ctx->token->text, require) != 0) {
// 		if (after) {
// 			E_ERROR(ctx, ctx->token, "requires '%s' after '%s'", require, after);
// 		} else {
// 			E_ERROR(ctx, ctx->token, "requires '%s'", require);
// 		}
// 	} else {
// 		rh_next_token(ctx);
// 	}
// }
// 
// rh_asm_statment *rh_compile_statment(rh_context *ctx, int level) {
// 	rh_asm_statment *statment = malloc(sizeof(rh_asm_statment));
// 	rh_type *type;
// 	statment->next = NULL;
// 	if (strcmp(ctx->token->text, "if") == 0) {
// 		statment->type = STAT_IF;
// 		rh_next_token(ctx);
// 		error_with_token(ctx, "(", "if");
// 		statment->exp[0] = rh_compile_exp(ctx);
// 		error_with_token(ctx, ")", 0);
// 		statment->statment[0] = rh_compile_statment(ctx, level);
// 		if (strcmp(ctx->token->text, "else") == 0) {
// 			rh_next_token(ctx);
// 			statment->statment[1] = rh_compile_statment(ctx, level);
// 		} else statment->statment[1] = NULL;
// 	} else if (strcmp(ctx->token->text, "{") == 0) {
// 		statment->type = STAT_COMPOUND;
// 		rh_next_token(ctx);
// 		rh_asm_statment *st, *last = NULL;
// 		while (ctx->token->type != TKN_NULL && strcmp(ctx->token->text, "}") != 0) {
// 			st = rh_compile_statment(ctx, level + 1);
// 			if (last == NULL) {
// 				statment->statment[0] = last = st;
// 			} else {
// 				last->next = st;
// 			}
// 			last = st;
// 		}
// 		error_with_token(ctx, "}", 0);
// 	} else if (strcmp(ctx->token->text, ";") == 0) {
// 		statment->type = STAT_BLANK;
// 	} else if ((type = get_type(ctx))->kind != TK_NULL) {
// 		statment->type = STAT_BLANK;
// 		rh_type *tp = ctx->type_top;
// 		while (tp) {
// 			if (tp->level != level || strcmp(tp->token->text, type->token->text) == 0) break;
// 			tp = tp->next;
// 		}
// 		if (tp && tp->level == level) {
// 			rh_free(type);
// 			E_ERROR(ctx, type->token, "The name '%s' already exists.", type->token->text);
// 		} else {
// 			type->next = ctx->type_top;
// 			type->level = level;
// 			ctx->type_top = type;
// 		}
// 		error_with_token(ctx, ";", 0);
// 	} else {
// 		rh_free(type);
// 		statment->type = STAT_EXPRESSION;
// 		statment->exp[0] = rh_compile_exp(ctx);
// 		error_with_token(ctx, ";", 0);
// 	}
// 	return statment;
// }
// 
// rh_asm_global *rh_compile_global(rh_context *ctx) {
// 	rh_asm_global *global = rh_malloc(sizeof(rh_asm_global));
// 	//global.exp = rh_compile_exp(ctx);
// 	global->statment = NULL;
// 	rh_asm_statment *st, *last;
// 	while (ctx->token->type != TKN_NULL) {
// 		st = rh_compile_statment(ctx, 0);
// 		if (global->statment == NULL) global->statment = st;
// 		else last->next = st;
// 		last = st;
// 	}
// 	return global;
// }
// 
// rh_asm_global *rh_compile(rh_context *ctx) {
// 	return rh_compile_global(ctx);
// }

/* vim: set foldmethod=marker : */
