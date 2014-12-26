#include "game_input.h"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

static const char *dev = "/dev/input/event1";

static ssize_t n;
static int fd;

static fd_set writefds;
static struct timeval timeout;

void setup_input() {
    fd = open(dev, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open %s: %s.\n", dev, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int get_key_event(struct input_event *ev) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 10;

    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    int result = select(fd+1, &writefds, NULL, NULL, &timeout);
    // Error!
    if (result == -1) return 0;
    else if (result) {
        n = read(fd, ev, sizeof *ev);
        // If it's got a wrong size of some other reason, we need to quit
        if (n == (ssize_t)-1) return 0;
        // If the event struct is the wrong size, we need to quit
        else if (n != sizeof *ev) return 0;
        return 1;
    }
    // There are no keypresses ready to be read
    return 0;
}

#define MAX_KEY 256
int direction_pressed[MAX_KEY];

int key_down(int direction) {
    if (direction < 0 || direction >= MAX_KEY) return 0;
    return direction_pressed[direction];
}

void handle_input() {
    struct input_event ev;
    while (get_key_event(&ev)) {
        if (ev.type == EV_KEY && ev.value == 0 || ev.value == 1) {
            direction_pressed[ev.code] = ev.value;
        }
    }
}