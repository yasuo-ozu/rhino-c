#include "common.h"

int rhino_main(int argc, char **argv) {
	char *fname = 0;
	int i = 1, f_help = 0, f_dump_token = 0;
	rh_context ctx;

	/* Process argument and option(s) */
	while (i < argc) {
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'h') {
				f_help = 1;
			} else if (argv[i][1] == 'd') {
				f_dump_token = 1;
			} else {
				printf("Invalid option : %s\n", argv[i]);
				return (1);
			}
			i++;
		} else {
			if (fname) {
				printf("Cannot load multiple files\n");
				return (1);
			}
			fname = argv[i];
			i++;
		}
	}

	/* Show help */
	if (!fname || f_help) {
		printf("Usage: %s file.c\n", argv[0]);
		return (1);
	}

	if (rh_error_init(&ctx.error, "ja_JP.UTF8")) {
		fprintf(stderr, "*** Stop.\n");
		rh_error_dump(&ctx.error, stderr);
		exit(1);
	}

	/* Load file */
	ctx.file.line = ctx.file.ch = 0;
	ctx.file.dump_token = f_dump_token;
	ctx.file.unget_buf_top = 0;
	strncpy(ctx.file.name, fname, MAX_FNAME);
	ctx.file.fp = fopen(fname, "r");
	if (!ctx.file.fp) {
		fprintf(stderr, "File open error: %s\n", fname);
		return (1);
	}

	/* compile */
	rh_token_init();
	rh_asm_global global;
	ctx.compile.token = rh_next_token(&ctx);
	global = rh_compile(&ctx);
	fclose(ctx.file.fp);
	rh_error_dump(&ctx.error, stderr);

	/* run */
	int ret;
	printf("Program ended with %d\n",  ret = rh_execute(&global));

	// TODO: release rh_asm_exp s

	return (ret);
}

int main(int argc, char **argv) {
	return rhino_main(argc, argv);
}

