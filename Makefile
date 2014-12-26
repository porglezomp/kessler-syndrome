LIBS = $(shell pkg-config PiGL --libs) -lm
CFLAGS = $(shell pkg-config PiGL --cflags)
SRC = game.c game_input.c
OBJS = game.o game_input.o
APP = Game.out

$(APP): $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)

clean:
	rm $(OBJS)
	rm $(APP)
