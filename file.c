#include "common.h"

void rh_ungetc(rh_file *file, int c) {
	if (c == -1) {
		file->unget_buf_top = 1;
		file->unget_buf[0] = -1;
	} else {
		file->unget_buf[file->unget_buf_top++] = c;
	}
}

int rh_getc(rh_file *file) {
	if (file->unget_buf_top) {
		return file->unget_buf[--file->unget_buf_top];
	}
	return fgetc(file->fp);
}

int rh_getchar(rh_file *file, int in_literal) {
	int c = rh_getc(file), a, b;
	if (c == -1) return -1;
	// TODO: insert proceedures for Trigraph, Preprocessor command
	if (!in_literal && c == '/') {
		if ((a = rh_getc(file)) == '*') {
			while ((a = rh_getc(file)) != EOF) {
				if (a == '*') {
					if ((a = rh_getc(file)) == '/') {
						c = ' '; break;
					} else {
						rh_ungetc(file, a);
					}
				}
			}
			if (a == EOF) {
				fprintf(stderr, "err: File reached EOF in comment\n");
				exit(1);
			}
		} else if (a == '/') {
			while ((c = rh_getc(file)), (c != EOF && c != '\n'));
		} else {
			rh_ungetc(file, a);
		}
	}
	return c;
}

/* vim: set foldmethod=marker : */
