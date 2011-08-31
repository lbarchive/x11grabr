CFLAGS  += -g 
LDFLAGS += -lX11 -lXext -lXfixes

all: x11grabr

x11grabr: x11grabr.o

x11grabr.o: x11grabr.c x11grabr.h

clean:
	rm -f x11grabr x11grabr.o

.PHONY: clean
