CC := g++

EXTRA_WARNINGS := -Wextra -ansi -std=c++98 -Wno-unused-parameter

GST_LIBS := $(shell pkg-config --libs gstreamer-0.10 gstreamer-interfaces-0.10)
GST_CFLAGS := $(shell pkg-config --cflags gstreamer-0.10 gstreamer-interfaces-0.10)
GTK_LIBS := $(shell pkg-config --libs gtk+-2.0)
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-2.0)

CFLAGS := -ggdb -Wall $(EXTRA_WARNINGS)

player: ui.o backend.o player.o main.o gstreamercommands.o
player: CFLAGS := $(CFLAGS) $(GTK_CFLAGS) $(GST_CFLAGS)
player: LIBS := $(LIBS) $(GTK_LIBS) $(GST_LIBS)
binaries += player

all: $(binaries)

$(binaries):
	$(CC) $(LDFLAGS) $(LIBS) -o $@ $^

%.o: %.cpp
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(binaries)
	find . -name "*.o" | xargs rm -rf
