LIBS = $(shell pkg-config PiGL --libs) -lm -leasyinput
CFLAGS = $(shell pkg-config PiGL --cflags) --std=gnu99 -fgnu89-inline -g
SRCS = game.c vec2.c ship.c particles.c space.c
OBJS = game.o vec2.o ship.o particles.o space.o
APP = Game.out

$(APP): $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(APP)
