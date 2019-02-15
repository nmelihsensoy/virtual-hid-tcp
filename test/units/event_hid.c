#include <stdio.h> 
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/input.h>


int main(int argc, char* argv[]){

    int fd = -1;
    int rd;
    char event[256];
    struct input_event ev[64];
    int size = sizeof(struct input_event);
    fd_set set;

    fd = open(argv[1], O_RDONLY);
    //ioctl(fd, EVIOCGRAB, 1);

	FD_ZERO(&set);
	FD_SET(fd, &set);

	while (1) {
		select(fd + 1, &set, NULL, NULL, NULL);

		rd = read(fd, ev, sizeof(ev));

		if (rd < (int) sizeof(struct input_event)){
			printf("expected %d bytes, got %d\n", (int) sizeof(struct input_event), rd);
			perror("\nevtest: error reading");
			return 1;
		}

		for (int i = 0; i < rd / sizeof(struct input_event); i++){
			printf ("%s: Type[%d] Code[%d] Value[%d]\n", argv[i+1], ev[i].type, ev[i].code, ev[i].value);
		}

	}
    
    //ioctl(fd, EVIOCGRAB, 0);
    close(fd);
    return 0;
}