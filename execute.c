#include "common.h"

// 本来式オブジェクトを返すべき
int rh_execute_exp(rh_asm_exp *exp) {
	if (exp->children == 0) {
		rh_asm_exp_literal *exp_literal = (rh_asm_exp_literal *) exp;
		return exp_literal->intval;
	} else {
		rh_asm_exp_terms *exp_terms = (rh_asm_exp_terms *) exp;
		if (exp_terms->token->text[0] == '+') return rh_execute_exp(exp_terms->items[0]) + rh_execute_exp(exp_terms->items[1]);
		else if (exp_terms->token->text[0] == '-') return rh_execute_exp(exp_terms->items[0]) - rh_execute_exp(exp_terms->items[1]);
		else if (exp_terms->token->text[0] == '*') return rh_execute_exp(exp_terms->items[0]) * rh_execute_exp(exp_terms->items[1]);
		else if (exp_terms->token->text[0] == '/') return rh_execute_exp(exp_terms->items[0]) / rh_execute_exp(exp_terms->items[1]);
		else if (exp_terms->token->text[0] == '%') return rh_execute_exp(exp_terms->items[0]) % rh_execute_exp(exp_terms->items[1]);
		else {
			fprintf(stderr, "Not implemented ");
			rh_dump_token(stdout, exp_terms->token);
			return 1;
		}
	}
}

int rh_execute(rh_asm_global *global) {

	return rh_execute_exp(global->exp);
}


/* vim: set foldmethod=marker : */
