#include "common.h"

int rhino_main(int argc, char **argv) {
	char *fname = 0;
	int i = 1, f_help = 0, f_dump_token = 0;
	rh_context ctx;

	ctx.file = NULL;
	ctx.token = NULL;

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
	ctx.file = malloc(sizeof(rh_file));
	ctx.file->line = ctx.file->ch = 0;
	ctx.file->dump_token = f_dump_token;
	ctx.file->unget_buf_top = 0;
	strncpy(ctx.file->name, fname, MAX_FNAME);
	ctx.file->fp = fopen(fname, "r");
	if (!ctx.file->fp) {
		E_FATAL(&ctx, 0, "File open error: %s\n", fname);
	}

	//rh_token_init();
	//rh_next_token(&ctx);
	// while (ctx.token->type != TKN_NULL) rh_next_token(&ctx);
	

	/* compile */
	rh_token_init();
	rh_asm_global *global;
	E_NOTICE(&ctx, 0, "Begin compile...\n");
	rh_next_token(&ctx);
	global = rh_compile(&ctx);
	fclose(ctx.file->fp);
	rh_error_dump(&ctx.error, stderr);

	E_NOTICE(&ctx, 0, "Begin execute...\n");
	// /* run */
	 int ret;
	 printf("Program ended with %d\n",  ret = rh_execute(&ctx, global));

	// TODO: release rh_asm_exp s

	 return (ret);
}

int main(int argc, char **argv) {
	return rhino_main(argc, argv);
}

