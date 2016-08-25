SRCS = file.c main.c token.c execute.c memory.c table.c error.c type.c
HEADERS = common.h

.PHONY: clean unix dos score

rhino:	$(SRCS) $(HEADERS) Makefile
	gcc -O2 -o rhino $(SRCS)

clean:	
	rm -rf *.o rhino *~ .*~ *.swp .*.swp Session.vim *.exe Debug scsc

unix:
	dos2unix *.c *.h
	
dos:
	unix2dos *.c *.h

scsc:	scsc.c
	gcc scsc.c -o scsc
score:	scsc ${SRCS} ${HEADERS} rhino
	@echo "## SCSC Score:"
	@./scsc ${SRCS} ${HEADERS}
	@echo "## Char count:" `(LANG=C;wc -m ${SRCS} ${HEADERS}) | sed -n -e '/total/p'`
	@echo "## Line count:" `(LANG=C;wc -l ${SRCS} ${HEADERS}) | sed -n -e '/total/p'`
	@strip rhino
	@echo "## Binary file size:" `du -b rhino | sed -e 's/^\([^\s]*\)\srhino/\1/'`
	upx -9 rhino
	@echo "## Compressed binary file size:" `du -b rhino | sed -e 's/^\([^\s]*\)\srhino/\1/'`
	@rm rhino

# vim: set ts=8 sw=8 :
