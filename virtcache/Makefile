# Set project version
ifndef PROJECT_VERSION:
PROJECT_VERSION := git-$(shell git rev-parse HEAD 2>/dev/null)
endif

CC            := clang
CFLAGS        := -std=gnu11 -DPROJECT_VERSION='"$(PROJECT_VERSION)"' \
 -Wall -Wextra -Wpedantic \
 -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused \
 -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference \
 -Wdouble-promotion -Wformat=2 -g
INCL          := -I./include
EXT           := cpp
LIBS          := -L./lib -lvirtcache -lrt -Wl,-rpath,./lib


.PHONY: all
all: lib prog


.PHONY: lib
lib:
	$(CC) $(CFLAGS) $(INCL) -shared -fpic -o ./lib/libvirtcache.so src/virtcache.c
	

.PHONY: prog
prog:
	$(CC) $(CFLAGS) $(INCL) src/main_viewer.c -o viewer $(LIBS)
	$(CC) $(CFLAGS) $(INCL) src/main_setter.c -o setter  $(LIBS)
	$(CC) $(CFLAGS) $(INCL) src/main_flush.c -o flush  $(LIBS)
	$(CC) $(CFLAGS) $(INCL) exercise_1_victim.c -o exercise_1_victim $(LIBS)
	$(CC) $(CFLAGS) $(INCL) exercise_1_attacker.c -o exercise_1_attacker $(LIBS)

.PHONY: clean
clean:
	rm -rf ./build
	rm -rf ./lib/libvirtcache.so
	rm -f ./main ./viewer ./setter ./flush ./exercise_1_attacker ./exercise_1_victim

