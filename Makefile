cc=gcc
opt=-o myServer
obj=hw3.c  handler.c
all: 
	$(cc) $(obj) $(opt) 
run:
	$(CURDIR)/myServer
clean:
	rm myServer
