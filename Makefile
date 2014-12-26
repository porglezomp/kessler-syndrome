LIBS = $(shell pkg-config PiGL --libs) -lm
CFLAGS = $(shell pkg-config PiGL --cflags) --std=gnu99 -fgnu89-inline
SRC = game.c game_input.c vec2.c
OBJS = game.o game_input.o vec2.o
APP = Game.out

$(APP): $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(APP)
