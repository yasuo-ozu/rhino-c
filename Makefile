SRCS = compile.c file.c main.c token.c execute.c memory.c table.c error.c type.c
HEADERS = *.h

.PHONY: clean unix dos

rhino:	$(SRCS) $(HEADERS) Makefile
	gcc -O2 -o rhino $(SRCS)

clean:	
	rm -rf *.o rhino

unix:
	dos2unix *.c *.h
	
dos:
	unix2dos *.c *.h

# vim: set ts=8 sw=8 :
