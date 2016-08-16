#include "common.h"

int rhino_main(int argc, char **argv) {
	char *fname = 0;
	int i = 1, f_help = 0, f_dump_token = 0;

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

	/* Load file */
	rh_file file;
	file.line = file.ch = 0;
	file.dump_token = f_dump_token;
	file.unget_buf_top = 0;
	strncpy(file.name, fname, MAX_FNAME);
	file.fp = fopen(fname, "r");
	if (!file.fp) {
		fprintf(stderr, "File open error: %s\n", fname);
		return (1);
	}

	rh_token token;
	// Normally rh_next_token will be called from something compile proceedures.
	//while ((token = rh_next_token(&file)).kind);

	int c;
	while ((c = rh_getchar(&file, 0)) != EOF) {
		printf("%c", (char) c);
	}

	fclose(file.fp);
	return (0);
}

int main(int argc, char **argv) {
	return rhino_main(argc, argv);
}

