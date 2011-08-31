CFLAGS  += -g 
LDFLAGS += -lavcodec -lavformat  -lX11 -lXext -lXfixes

all: x11grabr

x11grabr: x11grabr.o

x11grabr.o: x11grabr.c

clean:
	rm -f x11grabr x11grabr.o

.PHONY: clean
