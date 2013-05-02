cc=gcc
opt=-o myServer
obj=hw2.c  handler.c
all: 
	$(cc) $(obj) $(opt) 
run:
	$(CURDIR)/myServer
clean:
	rm myServer
