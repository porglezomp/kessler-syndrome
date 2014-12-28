LIBS = $(shell pkg-config PiGL --libs) -lm -leasyinput
CFLAGS = $(shell pkg-config PiGL --cflags) --std=gnu99 -fgnu89-inline -g -Wall -Werror
SRCS = game.c vec2.c rocket.c particles.c space.c gui.c rigidbody.c camera.c
OBJS = game.o vec2.o rocket.o particles.o space.o gui.o rigidbody.o camera.o
APP = Game.out

$(APP): $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(APP)
