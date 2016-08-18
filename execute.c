#include "common.h"

// int rh_execute_exp(rh_asm_exp *exp) {
// 	if (exp->token.kind == TK_VAL) return exp->token.val_int;
// 	else if (exp->token.kind == TK_PLUS) return rh_execute_exp(exp->arg1) + rh_execute_exp(exp->arg2);
// 	else if (exp->token.kind == TK_MINUS) return rh_execute_exp(exp->arg1) - rh_execute_exp(exp->arg2);
// 	else if (exp->token.kind == TK_MUL) return rh_execute_exp(exp->arg1) * rh_execute_exp(exp->arg2);
// 	else if (exp->token.kind == TK_DIV) return rh_execute_exp(exp->arg1) / rh_execute_exp(exp->arg2);
// 	else {
// 		fprintf(stderr, "Not implemented ");
// 		rh_dump_token(stdout, exp->token);
// 		//exit(1);
// 		return 1;
// 	}
// }
// 
// int rh_execute(rh_asm_global *global) {
// 
// 	return rh_execute_exp(global->exp);
// }


/* vim: set foldmethod=marker : */
