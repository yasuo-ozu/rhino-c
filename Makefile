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
	./scsc ${SRCS} ${HEADERS}
	ls -l rhino

# vim: set ts=8 sw=8 :
