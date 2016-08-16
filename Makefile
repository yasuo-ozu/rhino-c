SRCS = *.c
HEADERS = *.h

.PHONY: clean

rhino:	$(SRCS) $(HEADERS) Makefile
	gcc -Wall -O2 -o rhino $(SRCS)

clean:	
	rm -rf *.o rhino

	

# vim: set ts=8 sw=8 :
