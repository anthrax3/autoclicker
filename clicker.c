#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "clicker.h"


Display* openDisplay(const char *display) {
    Display *d;

    if((d = XOpenDisplay(display)) == NULL) {
        fprintf(stderr, "Can't open display <%s>\n", display);
        exit(EXIT_FAILURE);
    }

    return d;
}

void fillEvent(Display *display, XEvent *ev) {
    memset(ev, 0, sizeof(*ev));

    ev->xbutton.button = Button1;
    ev->xbutton.same_screen = True;
    ev->xbutton.subwindow = DefaultRootWindow(display);

    while(ev->xbutton.subwindow) {
        ev->xbutton.window = ev->xbutton.subwindow;
        XQueryPointer(display, ev->xbutton.window,
                      &ev->xbutton.root,
                      &ev->xbutton.subwindow,
                      &ev->xbutton.x_root,
                      &ev->xbutton.y_root,
                      &ev->xbutton.x,
                      &ev->xbutton.y,
                      &ev->xbutton.state);
    }
}

void fillEventPress(XEvent *ev) {
    ev->type = ButtonPress;
}

void fillEventRelease(XEvent *ev) {
    ev->type = ButtonRelease;
}

void autoClic(Options *opts) {
    Display *display;
    XEvent event;
    int i;

    display = openDisplay(NULL);

    if(opts->time > 0) {
        printf("Clicking will start in %d seconds\n", opts->time);
        sleep(opts->time);
    }

    fillEvent(display, &event);
    printf("Starting clicking on current window at (%d, %d)\n", event.xbutton.x, event.xbutton.y);

    if(opts->clics == 0) {
        printf("Number of clics: infinite\n");
    } else {
        printf("Number of clics: %d\n", opts->clics);
    }

    printf("Time between clics: %d ms\n", opts->ms);


    for(i = 0; i < opts->clics || !opts->clics; i++) {
        fillEventPress(&event);

        if(XSendEvent(display, PointerWindow, True, ButtonPressMask, &event) == 0) {
            fprintf (stderr, "Error during sending the event\n");
            exit(EXIT_FAILURE);
        }

        XFlush(display);

        if(opts->ms > 0) {
            usleep(opts->ms*1000);
        }

        fillEventRelease(&event);

        if(XSendEvent(display, PointerWindow, True, ButtonPressMask, &event) == 0) {
            fprintf (stderr, "Error during sending the event\n");
            exit(EXIT_FAILURE);
        }

        XFlush(display);

    }
}

void usage(const char *progname) {
    printf("Usage : %s [OPTIONS]\n", progname);
    printf("   OPTIONS :\n");
    printf("     -c <n>      Number of clics (0 means infinite)\n");
    printf("     -m <ms>     Number of milliseconds between each clics (min. 50ms)\n");
    printf("     -t <s>      Number of seconds before starting clicking\n");
    printf("     -h          Print help\n");
}

void parseOptions(int argc, char **argv, Options *opts) {
    int o;

    memset(opts, 0, sizeof(*opts));

    while((o = getopt(argc, argv, "c:m:t:h")) != -1) {
        switch(o) {

        case 'c':
            opts->clics = atoi(optarg);
            break;

        case 'm':
            opts->ms = atoi(optarg);
            break;

        case 't':
            opts->time = atoi(optarg);
            break;

        case 'h':
            usage(argv[0]);
            exit(EXIT_FAILURE);
            break;

        default:
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if(opts->ms < 50)
        opts->ms = 50;

    if(opts->time < 0)
        opts->time = 0;

    if(opts->clics < 0)
        opts->clics = 0;
}

int main(int argc, char **argv) {
    Options opts;

    parseOptions(argc, argv, &opts);
    autoClic(&opts);

    return EXIT_SUCCESS;
}
