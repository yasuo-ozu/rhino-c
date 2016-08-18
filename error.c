#include "common.h"
#include <stdarg.h>
#include <setjmp.h>


void rh_error_dump(rh_error_context *err, FILE *fp) {
	int i, j, k, c, p;
	for (i = 0; i < err->errors; i++) {
		fprintf(fp, "% 4d,% 3d  ", err->error[i].line1, err->error[i].ch1);
		if (err->error[i].type == ETYPE_NOTICE)		fprintf(fp, "NOTICE: "); p = 8;
		if (err->error[i].type == ETYPE_WARNING)	fprintf(fp, "WARNING: "); p = 9;
		if (err->error[i].type == ETYPE_ERROR)		fprintf(fp, "ERROR: "); p = 7;
		if (err->error[i].type == ETYPE_FATAL)		fprintf(fp, "FATAL: "); p = 7;
		if (err->error[i].type == ETYPE_TRAP)		fprintf(fp, "TRAP: "); p = 6;
		if (err->error[i].type == ETYPE_INTERNAL)	fprintf(fp, "INTERNAL: "); p = 10;
		p += 10;
		for (j = 0; c = err->error[i].message[j]; j++) {
			if (j && j % (TERM_MAX_WIDTH - p) == 0) {
				fprintf(fp, "\n");
				for(k = 0; k < p; k++) fprintf(fp, " ");
			}
			fputc(c, fp);
		}
	}
	fprintf(fp, "\n");
}
/*
int rh_error_init(rh_error_context *err, char *lang) {
	err->errors = 0;
	if (setjmp(err->jmpbuf)) {
		rh_error_dump(err, stderr);
		exit(1);
	}
	return 0;

}
*/
void rh_error(rh_context *ctx, rh_error_type type, rh_token *token, char *msg, ...) {
	va_list va;
	int i, count = 0;
	rh_error_context_item *err = &ctx->error.error[ctx->error.errors];
	va_start(va, msg);
	if (msg != NULL) {	/* direct describe mode */
		vsnprintf(ctx->error.error[ctx->error.errors].message, ERROR_MAX_LENGTH, msg, va);
	} else {	/* error number mode */
		// TODO:
		*ctx->error.error[ctx->error.errors].message = '\0';
	}
	if (token != NULL) {
		err->line1 = token->line1;
		err->line2 = token->line2;
		err->ch1 = token->ch1;
		err->ch2 = token->ch2;
	} else {
		if (ctx->file != NULL) {
			err->line1 = err->line2 = ctx->file->line;
			err->ch1 = err->ch2 = ctx->file->ch;
		} else {
			err->line1 = err->line2 = -1;
			err->ch1 = err->ch2 = -1;
		}
	}
	err->type = type;
	ctx->error.errors++;
	if (type == ETYPE_ERROR) {
		for (i = 0; i < ctx->error.errors; i++)
			if (ctx->error.error[i].type == ETYPE_ERROR) count++;
	}
	if (type == ETYPE_FATAL || count >= ERROR_THRESHOLD) {
		longjmp(ctx->error.jmpbuf, 1);
	}
	va_end(va);
}



/* vim: set foldmethod=marker : */
