LIBS = $(shell pkg-config PiGL --libs) -lm -leasyinput
# Use -MMD to generate dependency files
CFLAGS = $(shell pkg-config PiGL --cflags) --std=gnu99 -fgnu89-inline -g -Wall -Werror -MMD
SRCS = game.c vec2.c rocket.c particles.c space.c gui.c rigidbody.c camera.c
OBJS = $(SRCS:%.c=%.o)
DEPS = $(SRCS:%.c=%.d)
APP = Game.out

$(APP): $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

%.o: %.c
	gcc -c $< $(CFLAGS)

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(APP)

test: test/Test.out

test/Test.out: test/test.c vec2.o
	gcc -o test/Test.out -I./ test/test.c vec2.o -lm
	./test/Test.out

# Include the generated dependency files for smarter rebuilds
-include $(DEPS)
