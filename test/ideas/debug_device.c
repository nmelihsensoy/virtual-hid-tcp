#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

struct event_device{
    char *device;
    int fd;
};

int main(int argc, char* argv[]){
    struct input_event ev[64];
    int numevents;
    int result = 0;
    int size = sizeof(struct input_event);
    int rd;
    char name[256];
    char* device[12];
    struct event_device evdevs[12], *evdev;
    int numevdevs = 0;
    fd_set fds;
    int maxfd;

    device[0] = "/dev/input/event11";
    device[1] = "/dev/input/event18";

    for (int i = 0; i < 1; ++i){
        evdev = &evdevs[numevdevs];

        evdev->device = device[i];
        evdev->fd = open(evdev->device, O_RDONLY);
        if (evdev->fd == -1) {
            printf("Failed to open event device: %s.\n", evdev->device);
            continue;
        }
        ++numevdevs;

        memset(name, 0, sizeof(name));
        result = ioctl(evdev->fd, EVIOCGNAME(sizeof(name)), name);
        printf ("Reading From : %s (%s)\n", evdev->device, name);

        printf("Getting exclusive access: ");
        result = ioctl(evdev->fd, EVIOCGRAB, 1);
        printf("%s\n", (result == 0) ? "SUCCESS" : "FAILURE");
    }

    if (numevdevs == 0){
        exit(1);
    }

    while (1){
        FD_ZERO(&fds);
        maxfd = -1;

        for(int i = 0; i < numevdevs; ++i){
            evdev = &evdevs[i];
            FD_SET(evdev->fd, &fds);
            if (maxfd < evdev->fd) maxfd = evdev->fd;
        }

        result = select(maxfd+1, &fds, NULL, NULL, NULL);
        if(result == -1){
            break;        
        }

        for(int i = 0; i < numevdevs; ++i){
            evdev = &evdevs[i];

            if(!FD_ISSET(evdev->fd, &fds)){
                continue;
            }

            if ((rd = read(evdev->fd, ev, size * 64)) < size) {
                continue;
            }

            numevents = rd / size;
            for (int j = 0; j < numevents; ++j) {
                printf ("%s: Type[%d] Code[%d] Value[%d]\n", evdev->device, ev[j].type, ev[j].code, ev[j].value);
            }
        }
    }

    printf("Exiting.\n");

    for(int i = 0; i < numevdevs; ++i){
        evdev = &evdevs[i];
        result = ioctl(evdev->fd, EVIOCGRAB, 0);
        close(evdev->fd);
    }

    return 0;
}