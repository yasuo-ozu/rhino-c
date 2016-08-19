#include "common.h"

// 本来式オブジェクトを返すべき
int rh_execute_exp(rh_asm_exp *exp) {
	if (exp->type == EXP_LITERAL) {
		return exp->literal.intval;
	} else {
		if (exp->op.token->text[0] == '+') return rh_execute_exp(exp->op.exp[0]) + rh_execute_exp(exp->op.exp[1]);
		else if (exp->op.token->text[0] == '-') return rh_execute_exp(exp->op.exp[0]) - rh_execute_exp(exp->op.exp[1]);
		else if (exp->op.token->text[0] == '*') return rh_execute_exp(exp->op.exp[0]) * rh_execute_exp(exp->op.exp[1]);
		else if (exp->op.token->text[0] == '/') return rh_execute_exp(exp->op.exp[0]) / rh_execute_exp(exp->op.exp[1]);
		else if (exp->op.token->text[0] == '%') return rh_execute_exp(exp->op.exp[0]) % rh_execute_exp(exp->op.exp[1]);
		else {
			fprintf(stderr, "Not implemented ");
			rh_dump_token(stdout, exp->op.token);
			return 1;
		}
	}
}

int rh_execute(rh_asm_global *global) {

	return rh_execute_exp(global->exp);
}


/* vim: set foldmethod=marker : */
