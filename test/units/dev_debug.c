#include <stdio.h> 
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include <linux/input.h>

int device_count = 0;
char **device;

void error_handling(char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

void scan_devices(){
    struct dirent *de;
    int i=0;

    DIR *dr = opendir("/dev/input"); 
  
    if (dr == NULL){ 
        error_handling("Could not open directory");
    } 

    device = malloc(50 * sizeof(char*));

    while((de = readdir(dr)) != NULL){
        if(strncmp("event", de->d_name, 5) == 0){
            char filename[64];
            int fd = -1;
            char name[256] = "???";

            sprintf(filename, "%s%s", "/dev/input/", de->d_name);

            fd = open(filename, O_RDONLY);
            if(fd < 0)
                continue;
            
            ioctl(fd, EVIOCGNAME(sizeof(name)), name);            
            printf("%i - (%s) %s\n", i, de->d_name, name);
            device[i] = malloc(100);
            strncpy(device[i], filename, sizeof(filename));
            i++;
        }    
    }
    device_count = i;
    device = realloc(device,  (i)* sizeof(char*));
    closedir(dr); 
}

void print_events(char* device){
    int fd = -1;
    int rd;
    struct input_event ev[64];
    int size = sizeof(struct input_event);
    fd_set set;

    if((fd = open(device, O_RDONLY)) == 0){
        error_handling("cannot open device");
    }

    FD_ZERO(&set);
    FD_SET(fd, &set);

    while(1){
        select(fd+1, &set, NULL, NULL, NULL);
        rd = read(fd, ev, sizeof(ev));

        if(rd < (int)sizeof(struct input_event)){
            error_handling("read error");
        }

        for (int i = 0; i < rd / sizeof(struct input_event); i++){
			printf("%s: Type[%d] Code[%d] Value[%d]\n", device, ev[i].type, ev[i].code, ev[i].value);
		}
    
    }

    close(fd);
}

int main(){ 
    
    int choice;
    scan_devices();
    printf("Select Event: ");
    scanf("%d", &choice);
    print_events(*(device+choice));

    //printf("%s", *(device+choice));
        
    return 0; 
} 