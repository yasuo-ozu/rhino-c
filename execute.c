#include "common.h"

// 本来式オブジェクトを返すべき
rh_asm_exp *rh_execute_exp(rh_context *ctx, rh_asm_exp *exp) {
	if (exp->type == EXP_LITERAL) {
		return exp;
	} else if (exp->type == EXP_VARIABLE) {
		rh_asm_exp *ret = rh_malloc(sizeof(rh_asm_exp));
		ret->type = EXP_VARIABLE;
		ret->var = exp->var;
		ret->literal.intval = exp->var->intval;
		ret->literal.dblval = exp->var->dblval;
		return ret;
	} else {
		rh_asm_exp *exp1, *exp2, *ret = rh_malloc(sizeof(rh_asm_exp));
		ret->type = EXP_LITERAL;
		exp1 = rh_execute_exp(ctx, exp->op.exp[0]);
		exp2 = rh_execute_exp(ctx, exp->op.exp[1]);
		if (exp->op.token->text[0] == '+') {
			ret->literal.intval = exp1->literal.intval + exp2->literal.intval; 
			ret->literal.dblval = exp1->literal.dblval + exp2->literal.dblval; 
		} else if (exp->op.token->text[0] == '-') {
			ret->literal.intval = exp1->literal.intval - exp2->literal.intval; 
			ret->literal.dblval = exp1->literal.dblval - exp2->literal.dblval; 
		} else if (exp->op.token->text[0] == '*') {
			ret->literal.intval = exp1->literal.intval * exp2->literal.intval; 
			ret->literal.dblval = exp1->literal.dblval * exp2->literal.dblval; 
		} else if (exp->op.token->text[0] == '/') {
			ret->literal.intval = exp1->literal.intval / exp2->literal.intval; 
			ret->literal.dblval = exp1->literal.dblval / exp2->literal.dblval; 
		} else if (exp->op.token->text[0] == '%') {
			ret->literal.intval = exp1->literal.intval % exp2->literal.intval; 
			ret->literal.dblval = (int) exp1->literal.dblval % (int) exp2->literal.dblval; 
		} else if (exp->op.token->text[0] == '=') {
			ret->literal.intval = exp1->var->intval = exp2->literal.intval; 
			ret->literal.dblval = exp1->var->dblval = exp2->literal.dblval; 
		} else {
			fprintf(stderr, "Not implemented ");
			rh_dump_token(stdout, exp->op.token);
			return exp;
		}
	}
}

void rh_execute_statment(rh_context *ctx, rh_asm_statment *statment) {
	if (statment->type == STAT_IF) {
		if (rh_execute_exp(ctx, statment->exp[0])) 
			rh_execute_statment(ctx, statment->statment[0]);
		else if (statment->statment[1])
			rh_execute_statment(ctx, statment->statment[1]);
	} else if (statment->type == STAT_EXPRESSION) {
		printf("# %d\n", rh_execute_exp(ctx, statment->exp[0])->literal.intval);
	} else if (statment->type == STAT_COMPOUND) {
		rh_asm_statment *s = statment->statment[0];
		while (s) rh_execute_statment(ctx, s), s = s->next;
	} else if (statment->type == STAT_BLANK) {
		/* do nothing */
	} else {
		E_INTERNAL(ctx, 0, "Not implemented");
	}
}

int rh_execute(rh_context *ctx, rh_asm_global *global) {
	rh_asm_statment *statment = global->statment;
	while (statment) {
		rh_execute_statment(ctx, statment);
		statment = statment->next;
	}
	return 0;
}


/* vim: set foldmethod=marker : */
