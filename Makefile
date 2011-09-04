CFLAGS  += -g $(shell pkg-config --cflags cairo) 
LDFLAGS += -lX11 -lXext -lXfixes -lXtst -lgmp $(shell pkg-config --libs cairo) 

all: x11grabr

x11grabr: x11grabr.o options.o record.o util.o

x11grabr.o: x11grabr.c x11grabr.h options.h record.h

options.o: options.c options.h util.h

record.o: record.c record.h x11grabr.h

util.o: util.c util.h

clean:
	rm -f x11grabr x11grabr.o options.o record.o util.o

.PHONY: clean
