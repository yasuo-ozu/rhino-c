#include "common.h"

// 本来式オブジェクトを返すべき
int rh_execute_exp(rh_context *ctx, rh_asm_exp *exp) {
	if (exp->type == EXP_LITERAL) {
		return exp->literal.intval;
	} else {
		if (exp->op.token->text[0] == '+') return rh_execute_exp(ctx, exp->op.exp[0]) + rh_execute_exp(ctx, exp->op.exp[1]);
		else if (exp->op.token->text[0] == '-') return rh_execute_exp(ctx, exp->op.exp[0]) - rh_execute_exp(ctx, exp->op.exp[1]);
		else if (exp->op.token->text[0] == '*') return rh_execute_exp(ctx, exp->op.exp[0]) * rh_execute_exp(ctx, exp->op.exp[1]);
		else if (exp->op.token->text[0] == '/') return rh_execute_exp(ctx, exp->op.exp[0]) / rh_execute_exp(ctx, exp->op.exp[1]);
		else if (exp->op.token->text[0] == '%') return rh_execute_exp(ctx, exp->op.exp[0]) % rh_execute_exp(ctx, exp->op.exp[1]);
		else {
			fprintf(stderr, "Not implemented ");
			rh_dump_token(stdout, exp->op.token);
			return 1;
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
		printf("# %d\n", rh_execute_exp(ctx, statment->exp[0]));
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
