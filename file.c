#include "common.h"

void rh_ungetc(rh_file *file, int c) {
	if (c == -1) {
		file->unget_buf_top = 1;
		file->unget_buf[0] = -1;
		file->line = -1;
		file->ch = -1;
	} else {
		file->unget_buf[file->unget_buf_top++] = c;
		if (c == '\n') {
			~file->line && file->line--;
			file->ch = -1;
		}
	}
}

int rh_getc(rh_file *file) {
	int c;
	if (file->unget_buf_top) {
		c = file->unget_buf[--file->unget_buf_top];
	}
	else c = fgetc(file->fp);
	if (c == '\n') {
		~(file->line) && file->line++;
		~(file->ch) && (file->ch = 0);
	} else if (c == -1) {
		file->line = -1;
		file->ch = -1;
	} else {
		~(file->ch) && file->ch++;
	}
	return c;
}

int rh_getchar(rh_context *ctx, int in_literal) {
	int c = rh_getc(&ctx->file), a, b;
	if (c == -1) return -1;
	// TODO: insert proceedures for Trigraph, Preprocessor command
	if (!in_literal && c == '/') {
		if ((a = rh_getc(&ctx->file)) == '*') {
			while ((a = rh_getc(&ctx->file)) != EOF) {
				if (a == '*') {
					if ((a = rh_getc(&ctx->file)) == '/') {
						c = ' '; break;
					} else {
						rh_ungetc(&ctx->file, a);
					}
				}
			}
			if (a == EOF) {
				fprintf(stderr, "err: File reached EOF in comment\n");
				exit(1);
			}
		} else if (a == '/') {
			while ((c = rh_getc(&ctx->file)), (c != EOF && c != '\n'));
		} else {
			rh_ungetc(&ctx->file, a);
		}
	}
	return c;
}

/* vim: set foldmethod=marker : */
