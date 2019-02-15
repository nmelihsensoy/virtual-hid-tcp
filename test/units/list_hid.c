#include <stdio.h> 
#include <dirent.h> 
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/input.h>

int main(){ 
    struct dirent *de;

    DIR *dr = opendir("/dev/input"); 
  
    if (dr == NULL){ 
        printf("Could not open directory"); 
        return 0; 
    } 

    while ((de = readdir(dr)) != NULL){
        if(strncmp("event", de->d_name, 5) == 0){
            char filename[64];
            int fd = -1;
            char name[256] = "???";

            sprintf(filename, "%s%s", "/dev/input/", de->d_name);

            fd = open(filename, O_RDONLY);
            if(fd < 0)
                continue;
            
            ioctl(fd, EVIOCGNAME(sizeof(name)), name);            
            printf("%s  %s\n", name, de->d_name);

        }    
    }
  
    closedir(dr);     
    return 0; 
} 