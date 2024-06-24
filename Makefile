TARGET = main

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -I/usr/include/SDL2 -I/usr/include/SDL2/SDL_ttf
LDFLAGS = -lSDL2 -lSDL2_ttf -lSDL2_image

SRCS = main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

rebuild: clean all

run: $(TARGET)
	./$(TARGET) /usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf

.PHONY: all clean rebuild run
