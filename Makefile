LIBS = $(shell pkg-config PiGL --libs) -lm -leasyinput
# Use -MMD to generate dependency files
CFLAGS = $(shell pkg-config PiGL --cflags) --std=gnu99 -fgnu89-inline -g -Wall -Werror -MMD
SRCS = $(shell echo src/*.c)
OBJS = $(SRCS:src/%.c=build/%.o)
DEPS = $(SRCS:src/%.c=build/%.d)
APP = Game.out

$(APP): directories $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

build/%.o: src/%.c
	gcc -c $< $(CFLAGS) -o $@

directories:
	mkdir -p build

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(APP)

test: test/Test.out

test/Test.out: test/test.c build/vec2.o
	gcc -o test/Test.out -I./ test/test.c vec2.o -lm
	./test/Test.out

# Include the generated dependency files for smarter rebuilds
-include $(DEPS)
