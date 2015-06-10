LIBS = $(shell pkg-config PiGL --libs) -lm -leasyinput
NODEPS = $(shell pkg-config PiGL --cflags) --std=gnu99 -fgnu89-inline -g -Wall -Werror
# Use -MMD to generate dependency files
CFLAGS = $(NODEPS) -MMD
ODIR = build
SRCS = $(shell echo src/*.c)
OBJS = $(SRCS:src/%.c=$(ODIR)/%.o)
DEPS = $(SRCS:src/%.c=$(ODIR)/%.d)
APP = Game.out
TOOLS = tools/MeshEdit.out

DUMMY := $(shell mkdir -p $(ODIR))

$(APP): $(OBJS)
	gcc -o $(APP) $(OBJS) $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c
	gcc -c $< $(CFLAGS) -o $@

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f $(APP)
	rmdir $(ODIR)
	rm -f $(TOOLS)

test: test/Test.out

test/Test.out: test/test.c build/vec2.o
	gcc -o test/Test.out -I./ test/test.c vec2.o -lm
	./test/Test.out

tools: $(TOOLS)

tools/MeshEdit.out: tools/meshedit.c build/vec2.o src/vec2.h stretchy_buffer.h
	gcc build/vec2.o tools/meshedit.c -o tools/MeshEdit.out $(NODEPS) $(LIBS)

# Include the generated dependency files for smarter rebuilds
-include $(DEPS)
