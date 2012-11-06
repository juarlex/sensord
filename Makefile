PROG = sensord
SOURCE_DIR = src
SOURCE_FILES = $(SOURCE_DIR)/tsensor.c
SOURCE_FILES += $(SOURCE_DIR)/sensord.c
DEST_DIR = /usr/local/sbin

LDFLAGS += -lsqlite3

CFLAGS += -O2
CFLAGS += -Wall
CFLAGS += -Wmissing-declarations
CFLAGS += -Wstrict-prototypes -Wmissing-prototypes
CFLAGS += -Wshadow -Wpointer-arith -Wcast-qual
CFLAGS += -Wsign-compare

build:
	$(CC) -o $(PROG) $(SOURCE_FILES) $(CFLAGS) $(LDFLAGS)

clean:
	rm $(PROG)

install:
	cp -p $(PROG) $(DEST_DIR)
