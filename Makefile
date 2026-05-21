CC     = gcc
CFLAGS = -O3 -march=native -flto -ffast-math -Wall -Wextra -I.
LIBS   = -lm -lpthread -lSDL3

SRCS = cube_main.c \
       ISA/memory.c \
       ISA/parser.c \
       ISA/execution.c \
       core/engine.c \
       core/CPU_map.c

TARGET = vgpu

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET)
