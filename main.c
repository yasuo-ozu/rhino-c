#include "common.h"

int rhino_main(int argc, char **argv) {
	char *fname = 0;
	int i = 1, f_help = 0, f_dump_token = 0;
	rh_context ctx;

	ctx.file = NULL;
	ctx.token = NULL;
	// ctx.type_top = NULL;

	ctx.error.errors = 0;
	if (setjmp(ctx.error.jmpbuf)) {
		fprintf(stderr, "*** Stop.\n");
		rh_error_dump(&ctx.error, stderr);
		exit(1);
	}

	/* Process argument and option(s) */
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'h') {
				f_help = 1;
			} else if (argv[i][1] == 'd') {
				f_dump_token = 1;
			} else {
				E_FATAL(&ctx, 0, "Invalid option : %s\n", argv[i]);
			}
			i++;
		} else {
			if (fname) {
				E_FATAL(&ctx, 0, "Cannot load multiple files\n");
			}
			fname = argv[i];
			i++;
		}
	}

	/* Show help */
	if (!fname || f_help) {
		printf("Usage: %s file.c\n", argv[0]);
		return (0);
	}

	/* Load file */
	file_init(&ctx, fname);
	ctx.file->dump_token = f_dump_token;

	rh_token_init();
	//rh_next_token(&ctx);
	// while (ctx.token->type != TKN_NULL) rh_next_token(&ctx);
	

	/* compile */
	// rh_token_init();
	// rh_asm_global *global;
	// E_NOTICE(&ctx, 0, "Begin compile...\n");
	// rh_next_token(&ctx);
	// global = rh_compile(&ctx);
	// //fclose(ctx.file->fp);
	rh_token *token, *token_last;
	while ((token = rh_next_token(&ctx))->type != TKN_NULL) {
		if (ctx.token == NULL) {
			ctx.token = token_last = token;
		} else {
			token_last->next = token;
			token_last = token;
		}
	}
	token_last->next = NULL;

	// /* run */
	 int ret;
	 ret = rh_execute(&ctx);

	// TODO: release rh_asm_exp s

	rh_error_dump(&ctx.error, stderr);
	 return (ret);
}

int main(int argc, char **argv) {
	return rhino_main(argc, argv);
}

