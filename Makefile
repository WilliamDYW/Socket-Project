CFILES:= $(shell ls|grep -v functions|grep .c)
PROGS:=$(patsubst %.c,%,$(CFILES))

all: $(PROGS)

%:%.c
	$(CC) functions.h functions.c $< -o $@ 

clean:$(PROGS)
	rm $(PROGS)
