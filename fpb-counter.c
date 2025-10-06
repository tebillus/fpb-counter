#include <stdio.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/select.h>
struct termios oldt;
#endif

void set_raw_mode() {
#ifndef _WIN32
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
#endif
}

void reset_mode() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
}

void set_nonblocking(int enable) {
#ifndef _WIN32
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    if (flags == -1) return;
    flags = enable ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    fcntl(STDIN_FILENO, F_SETFL, flags);
#endif
}

double get_current_time_seconds() {
#ifdef _WIN32
    return GetTickCount() / 1000.0;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
#endif
}

int has_input() {
#ifdef _WIN32
    return kbhit();
#else
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    struct timeval tv = {0L, 0L};
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
#endif
}

char get_input() {
#ifdef _WIN32
    return getch();
#else
    return getchar();
#endif
}

int main() {
    double fpb;
    printf("Enter frames per beat at 24 fps: ");
    scanf("%lf", &fpb);
    if (fpb <= 0) {
        printf("Invalid input.\n");
        return 1;
    }

    double bpm = 1440.0 / fpb;

    printf("\nFrames per beat at 24 fps: %d\n", (int)fpb);
    printf("General BPM: %.2f\n\n", bpm);

    set_raw_mode();
    set_nonblocking(1);

    printf("Flashing multiples at the tempo. Press 'q' to quit.\n");

    double beat_time_sec = fpb / 24.0;
    double next_beat = get_current_time_seconds() + beat_time_sec;
    double current_multiple = fpb;

    while (1) {
        double now = get_current_time_seconds();
        if (now >= next_beat) {
            printf("%d\n", (int)current_multiple);
            current_multiple += fpb;
            next_beat += beat_time_sec;
        }

        if (has_input()) {
            char ch = get_input();
            if (ch == 'q' || ch == 'Q') {
                break;
            }
        }

#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }

    set_nonblocking(0);
    reset_mode();

    printf("\nPress any key to close...\n");
    set_raw_mode();
    get_input();
    reset_mode();

    return 0;
}
