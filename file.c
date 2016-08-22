#include "common.h"

// void rh_ungetc(rh_file *file, int c) {
// 	if (c == -1) {
// 		file->unget_buf_top = 1;
// 		file->unget_buf[0] = -1;
// 		file->line = -1;
// 		file->ch = -1;
// 	} else {
// 		file->unget_buf[file->unget_buf_top++] = c;
// 		if (c == '\n') {
// 			~file->line && file->line--;
// 			file->ch = -1;
// 		}
// 	}
// }

void file_init(rh_context *ctx, char *fname) {
	ctx->file = malloc(sizeof(rh_file));
	strncpy(ctx->file->name, fname, MAX_FNAME);
	FILE *fp = fopen(fname, "r");
	if (!fp) {
		E_FATAL(ctx, 0, "File open error: %s\n", fname);
	}
	ctx->buf = ctx->buf_end = rh_malloc(DEFAULT_FILE_BUF_SIZE);
	size_t buf_size = DEFAULT_FILE_BUF_SIZE;
	char *buf_max = ctx->buf + buf_size;
	int c;
	while((c = fgetc(fname)) != EOF) {
		if (c == 0) c = (int) ' ';
		if (ctx->buf_end + 16 >= buf_max) {
			size_t now = ctx->buf_end - ctx->buf;
			buf_size += DEFAULT_FILE_BUF_SIZE;
			ctx->buf = rh_realloc(ctx->buf, buf_size);
			ctx->buf_end = ctx->buf + now;
			buf_max = ctx_buf + buf_size;
		}
		*ctx->buf_end++ = (char) c;
	}
	ctx->ch = ctx->buf - (char *) 1;
	ctx->buf_end = 0;
	fclose(fp);
}

/* if ctx->ch is blank, returns 1. EOF then returns -1.*/
/* When result is 1, DO NOT use ctx->ch. */
int rh_nextchar(rh_context *ctx) {
	// TODO: insert proceedures for Trigraph, Preprocessor command
	// if (*ctx->ch == 0) return -1;
	// ctx->ch++;
	if (*ctx->ch == 0) return -1;
	else if (!in_literal && *ctx->ch == '/') {
		if (ctx->ch[1] == '*') {
			ctx->ch += 2;
			while (*ctx->ch) {
				if (*ctx->ch++ == '*') {
					if (*ctx->ch == '/') {
						return 1;
					}
				}
			}
			E_ERROR(ctx, 0, "File reached EOF in comment\n");
			return -1;
		} else if (ctx->ch[1] == '/') {
			while (*ctx->ch && *ctx->ch != '\n') ctx->ch++;
			return 1;
		}
	}
	return 0;
}

/* vim: set foldmethod=marker : */
