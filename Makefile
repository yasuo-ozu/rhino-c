SRCS = compile.c file.c main.c token.c
HEADERS = *.h

.PHONY: clean

rhino:	$(SRCS) $(HEADERS) Makefile
	gcc -O2 -o rhino $(SRCS)

clean:	
	rm -rf *.o rhino

	

# vim: set ts=8 sw=8 :
