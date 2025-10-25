# Set project version
ifndef PROJECT_VERSION:
PROJECT_VERSION := git-$(shell git rev-parse HEAD 2>/dev/null)
endif

CC            := clang
CFLAGS        := -std=gnu11 -DPROJECT_VERSION='"$(PROJECT_VERSION)"' \
 -Wall -Wextra -Wpedantic -Wno-overlength-strings \
 -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused \
 -Woverloaded-virtual -Wconversion -Wsign-conversion -Wnull-dereference \
 -Wdouble-promotion -Wformat=2 -Wno-strict-prototypes -g
INCL          := -I./include
EXT           := cpp
LIBS          := -L./lib -ldummy -Wl,-zlazy,-rpath,./lib

# Set debug flag or optimize
ifdef ENABLE_DEBUG
CXXFLAGS      += -g
else
CXXFLAGS      += -O3
endif

.PHONY: prog
prog:
	$(CC) $(CFLAGS) $(INCL) -no-pie -fno-pic exercise_sender.c -o exercise_sender $(LIBS)
	$(CC) $(CFLAGS) $(INCL) -no-pie -fno-pic exercise_receiver.c -o exercise_receiver $(LIBS)
	$(CC) $(CFLAGS) $(INCL) -no-pie -fno-pic exercise_plot.c -o exercise_plot $(LIBS)
	$(CC) $(CFLAGS) $(INCL) exercise_probe_close.c -o exercise_probe_close
	$(CC) $(CFLAGS) $(INCL) -no-pie -fno-pic exercise_flush_close.c -o exercise_flush_close

	@echo "\e[1mDid you disable powersave?: echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor\e[0m"

.PHONY: lib
lib:
	$(CC) $(CFLAGS) $(INCL) -shared -fpic -o ./lib/libdummy.so src/dummy.c
	

.PHONY: clean
clean:
	rm -rf ./build
	rm -rf ./lib/libdummy.so
	rm -f ./exercise_receiver ./exercise_sender ./exercise_plot ./exercise_probe_close ./exercise_flush_close

