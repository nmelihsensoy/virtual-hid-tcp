#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <termios.h>
#include <linux/uhid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h> //usleep, sleep, close, read, write 
#include <stdio.h> //printf, perror
#include <netinet/in.h> //htons, sockaddr_in
#include <string.h> //memset, strcpy, strlen
#include <fcntl.h> //open
#include <stdlib.h> //exit
#include <sys/socket.h>
#include <linux/uinput.h>

#define PORT 8080 //Socket port

struct uinput_setup usetup;

//Key events must be defined in keys[] before using
int keys[] = {BTN_LEFT, KEY_VOLUMEUP, KEY_VOLUMEDOWN}; 

int server_fd;
int new_socket, valread;
struct sockaddr_in address;
int opt = 1;
int addrlen = sizeof(address);


/*
 * Sends input events
*/
void emit(int fd, int type, int code, int val){
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    write(fd, &ie, sizeof(ie));
}

static unsigned char rdesc[] = {
	0x05, 0x01,                    // Usage Page (Generic Desktop)        0
    0x09, 0x02,                    // Usage (Mouse)                       2
    0xa1, 0x01,                    // Collection (Application)            4
    0x85, 0x01,                    //  Report ID (1)                      6
    0x09, 0x01,                    //  Usage (Pointer)                    8
    0xa1, 0x00,                    //  Collection (Physical)              10
    0x05, 0x09,                    //   Usage Page (Button)               12
    0x19, 0x01,                    //   Usage Minimum (1)                 14
    0x29, 0x08,                    //   Usage Maximum (8)                 16
    0x15, 0x00,                    //   Logical Minimum (0)               18
    0x25, 0x01,                    //   Logical Maximum (1)               20
    0x95, 0x08,                    //   Report Count (8)                  22
    0x75, 0x01,                    //   Report Size (1)                   24
    0x81, 0x02,                    //   Input (Data,Var,Abs)              26
    0x05, 0x01,                    //   Usage Page (Generic Desktop)      28
    0x16, 0x01, 0xf8,              //   Logical Minimum (-2047)           30
    0x26, 0xff, 0x07,              //   Logical Maximum (2047)            33
    0x75, 0x0c,                    //   Report Size (12)                  36
    0x95, 0x02,                    //   Report Count (2)                  38
    0x09, 0x30,                    //   Usage (X)                         40
    0x09, 0x31,                    //   Usage (Y)                         42
    0x81, 0x06,                    //   Input (Data,Var,Rel)              44
    0x15, 0x81,                    //   Logical Minimum (-127)            46
    0x25, 0x7f,                    //   Logical Maximum (127)             48
    0x75, 0x08,                    //   Report Size (8)                   50
    0x95, 0x01,                    //   Report Count (1)                  52
    0x09, 0x38,                    //   Usage (Wheel)                     54
    0x81, 0x06,                    //   Input (Data,Var,Rel)              56
    0xc0,                          //  End Collection                     58
    0xc0,                          // End Collection                      59
    0x05, 0x01,                    // Usage Page (Generic Desktop)        60
    0x09, 0x06,                    // Usage (Keyboard)                    62
    0xa1, 0x01,                    // Collection (Application)            64
    0x85, 0x08,                    //  Report ID (8)                      66
    0x05, 0x07,                    //  Usage Page (Keyboard)              68
    0x19, 0xe0,                    //  Usage Minimum (224)                70
    0x29, 0xe7,                    //  Usage Maximum (231)                72
    0x15, 0x00,                    //  Logical Minimum (0)                74
    0x25, 0x01,                    //  Logical Maximum (1)                76
    0x75, 0x01,                    //  Report Size (1)                    78
    0x95, 0x08,                    //  Report Count (8)                   80
    0x81, 0x02,                    //  Input (Data,Var,Abs)               82
    0x05, 0x07,                    //  Usage Page (Keyboard)              84
    0x19, 0x00,                    //  Usage Minimum (0)                  86
    0x29, 0x97,                    //  Usage Maximum (151)                88
    0x15, 0x00,                    //  Logical Minimum (0)                90
    0x25, 0x01,                    //  Logical Maximum (1)                92
    0x75, 0x01,                    //  Report Size (1)                    94
    0x96, 0x98, 0x00,              //  Report Count (152)                 96
    0x81, 0x02,                    //  Input (Data,Var,Abs)               99
    0xc0,                          // End Collection                      101
    0x06, 0x03, 0xff,              // Usage Page (Vendor Usage Page 0xff03) 102
    0x0a, 0x02, 0x24,              // Usage (Vendor Usage 0x2402)         105
    0xa1, 0x01,                    // Collection (Application)            108
    0x85, 0x03,                    //  Report ID (3)                      110
    0x19, 0x01,                    //  Usage Minimum (1)                  112
    0x29, 0x29,                    //  Usage Maximum (41)                 114
    0x15, 0x00,                    //  Logical Minimum (0)                116
    0x25, 0xff,                    //  Logical Maximum (255)              118
    0x75, 0x08,                    //  Report Size (8)                    120
    0x95, 0x29,                    //  Report Count (41)                  122
    0x81, 0x00,                    //  Input (Data,Arr,Abs)               124
    0x19, 0x01,                    //  Usage Minimum (1)                  126
    0x29, 0x1f,                    //  Usage Maximum (31)                 128
    0x15, 0x00,                    //  Logical Minimum (0)                130
    0x25, 0xff,                    //  Logical Maximum (255)              132
    0x75, 0x08,                    //  Report Size (8)                    134
    0x95, 0x29,                    //  Report Count (41)                  136
    0xb1, 0x02,                    //  Feature (Data,Var,Abs)             138
    0xc0,                          // End Collection                      140
    0x05, 0x0c,                    // Usage Page (Consumer Devices)       141
    0x09, 0x01,                    // Usage (Consumer Control)            143
    0xa1, 0x01,                    // Collection (Application)            145
    0x85, 0x04,                    //  Report ID (4)                      147
    0x19, 0x00,                    //  Usage Minimum (0)                  149
    0x2a, 0x9c, 0x02,              //  Usage Maximum (668)                151
    0x15, 0x00,                    //  Logical Minimum (0)                154
    0x26, 0x9c, 0x02,              //  Logical Maximum (668)              156
    0x75, 0x10,                    //  Report Size (16)                   159
    0x95, 0x01,                    //  Report Count (1)                   161
    0x81, 0x00,                    //  Input (Data,Arr,Abs)               163
    0xc0,                          // End Collection                      165
    0x05, 0x01,                    // Usage Page (Generic Desktop)        166
    0x09, 0x80,                    // Usage (System Control)              168
    0xa1, 0x01,                    // Collection (Application)            170
    0x85, 0x05,                    //  Report ID (5)                      172
    0x1a, 0x81, 0x00,              //  Usage Minimum (129)                174
    0x2a, 0x83, 0x00,              //  Usage Maximum (131)                177
    0x15, 0x00,                    //  Logical Minimum (0)                180
    0x25, 0x01,                    //  Logical Maximum (1)                182
    0x75, 0x01,                    //  Report Size (1)                    184
    0x95, 0x03,                    //  Report Count (3)                   186
    0x81, 0x02,                    //  Input (Data,Var,Abs)               188
    0x95, 0x05,                    //  Report Count (5)                   190
    0x81, 0x01,                    //  Input (Cnst,Arr,Abs)               192
    0xc0,                          // End Collection                      194
};


static int uhid_write(int fd, const struct uhid_event *ev)
{
	ssize_t ret;

	ret = write(fd, ev, sizeof(*ev));
	if (ret < 0) {
		fprintf(stderr, "Cannot write to uhid: %m\n");
		return -errno;
	} else if (ret != sizeof(*ev)) {
		fprintf(stderr, "Wrong size written to uhid: %ld != %lu\n",
			ret, sizeof(ev));
		return -EFAULT;
	} else {
		return 0;
	}
}

static int create(int fd)
{
	struct uhid_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_CREATE;
	strcpy((char*)ev.u.create.name, "test-uhid-device");
	ev.u.create.rd_data = rdesc;
	ev.u.create.rd_size = sizeof(rdesc);
	ev.u.create.bus = BUS_USB;
	ev.u.create.vendor = 0x15d9;
	ev.u.create.product = 0x0a37;
	ev.u.create.version = 0;
	ev.u.create.country = 0;

	return uhid_write(fd, &ev);
}

static void destroy(int fd)
{
	struct uhid_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_DESTROY;

	uhid_write(fd, &ev);
}

/* This parses raw output reports sent by the kernel to the device. A normal
 * uhid program shouldn't do this but instead just forward the raw report.
 * However, for ducomentational purposes, we try to detect LED events here and
 * print debug messages for it. */
static void handle_output(struct uhid_event *ev)
{
	/* LED messages are adverised via OUTPUT reports; ignore the rest */
	if (ev->u.output.rtype != UHID_OUTPUT_REPORT)
		return;
	/* LED reports have length 2 bytes */
	if (ev->u.output.size != 2)
		return;
	/* first byte is report-id which is 0x02 for LEDs in our rdesc */
	if (ev->u.output.data[0] != 0x2)
		return;

	/* print flags payload */
	fprintf(stderr, "LED output report received with flags %x\n",
		ev->u.output.data[1]);
}

static int event(int fd)
{
	struct uhid_event ev;
	ssize_t ret;

	memset(&ev, 0, sizeof(ev));
	ret = read(fd, &ev, sizeof(ev));
	if (ret == 0) {
		fprintf(stderr, "Read HUP on uhid-cdev\n");
		return -EFAULT;
	} else if (ret < 0) {
		fprintf(stderr, "Cannot read uhid-cdev: %m\n");
		return -errno;
	} else if (ret != sizeof(ev)) {
		fprintf(stderr, "Invalid size read from uhid-dev: %ld != %lu\n",
			ret, sizeof(ev));
		return -EFAULT;
	}

	switch (ev.type) {
	case UHID_START:
		fprintf(stderr, "UHID_START from uhid-dev\n");
		break;
	case UHID_STOP:
		fprintf(stderr, "UHID_STOP from uhid-dev\n");
		break;
	case UHID_OPEN:
		fprintf(stderr, "UHID_OPEN from uhid-dev\n");
		break;
	case UHID_CLOSE:
		fprintf(stderr, "UHID_CLOSE from uhid-dev\n");
		break;
	case UHID_OUTPUT:
		fprintf(stderr, "UHID_OUTPUT from uhid-dev\n");
		handle_output(&ev);
		break;
	case UHID_OUTPUT_EV:
		fprintf(stderr, "UHID_OUTPUT_EV from uhid-dev\n");
		break;
	default:
		fprintf(stderr, "Invalid event from uhid-dev: %u\n", ev.type);
	}

	return 0;
}

static bool btn1_down;
static bool btn2_down;
static bool btn3_down;
static signed char abs_hor;
static signed char abs_ver;
static signed char wheel;

static int send_event(int fd)
{
	struct uhid_event ev;

	memset(&ev, 0, sizeof(ev));
	ev.type = UHID_INPUT;
	ev.u.input.size = 3;

	ev.u.input.data[0] = 0x0;
	if (btn1_down)
		ev.u.input.data[0] = 0x1;
	
	//ev.u.input.data[0] = 0x1; left button
	//ev.u.input.data[0] = 0x3, 0x4; right button
	//ev.u.input.data[0] = 0x4; middle button
	if (btn2_down)
		ev.u.input.data[0] = 0x4;

	ev.u.input.data[1] = abs_hor;
	ev.u.input.data[2] = abs_ver;
	ev.u.input.data[4] = wheel;

	return uhid_write(fd, &ev);
}

static int keyboard(int fd)
{
	char buf[128];
	ssize_t ret, i;

	ret = read(STDIN_FILENO, buf, sizeof(buf));
	if (ret == 0) {
		fprintf(stderr, "Read HUP on stdin\n");
		return -EFAULT;
	} else if (ret < 0) {
		fprintf(stderr, "Cannot read stdin: %m\n");
		return -errno;
	}

	for (i = 0; i < ret; ++i) {
		switch (buf[i]) {
		case '1':
			btn1_down = !btn1_down;
			ret = send_event(fd);
			if (ret)
				return ret;
			break;
		case '2':
			btn2_down = !btn2_down;
			ret = send_event(fd);
			if (ret)
				return ret;
			break;
		case '3':
			btn3_down = !btn3_down;
			ret = send_event(fd);
			if (ret)
				return ret;
			break;
		case 'a':
			abs_hor = -20;
			ret = send_event(fd);
			abs_hor = 0;
			if (ret)
				return ret;
			break;
		case 'd':
			abs_hor = 20;
			ret = send_event(fd);
			abs_hor = 0;
			if (ret)
				return ret;
			break;
		case 'w':
			abs_ver = -20;
			ret = send_event(fd);
			abs_ver = 0;
			if (ret)
				return ret;
			break;
		case 's':
			abs_ver = 20;
			ret = send_event(fd);
			abs_ver = 0;
			if (ret)
				return ret;
			break;
		case 'r':
			wheel = 1;
			ret = send_event(fd);
			wheel = 0;
			if (ret)
				return ret;
			break;
		case 'f':
			wheel = -1;
			ret = send_event(fd);
			wheel = 0;
			if (ret)
				return ret;
			break;
		case 'q':
			return -ECANCELED;
		default:
			fprintf(stderr, "Invalid input: %c\n", buf[i]);
		}
	}

	return 0;
}

int main(int argc, char **argv){
	int fd;
	const char *path = "/dev/uhid";
	struct pollfd pfds[2];
	int ret;
	struct termios state;

	ret = tcgetattr(STDIN_FILENO, &state);
	if (ret) {
		fprintf(stderr, "Cannot get tty state\n");
	} else {
		state.c_lflag &= ~ICANON;
		state.c_cc[VMIN] = 1;
		ret = tcsetattr(STDIN_FILENO, TCSANOW, &state);
		if (ret)
			fprintf(stderr, "Cannot set tty state\n");
	}

	if (argc >= 2) {
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")) {
			fprintf(stderr, "Usage: %s [%s]\n", argv[0], path);
			return EXIT_SUCCESS;
		} else {
			path = argv[1];
		}
	}

	fprintf(stderr, "Open uhid-cdev %s\n", path);
	fd = open(path, O_RDWR | 0);
	if (fd < 0) {
		fprintf(stderr, "Cannot open uhid-cdev %s: %m\n", path);
		return EXIT_FAILURE;
	}

	fprintf(stderr, "Create uhid device\n");
	ret = create(fd);
	if (ret) {
		close(fd);
		return EXIT_FAILURE;
	}

	pfds[0].fd = STDIN_FILENO;
	pfds[0].events = POLLIN;
	pfds[1].fd = fd;
	pfds[1].events = POLLIN;

    //EVENT
    int fds = open("/dev/uinput", O_WRONLY | O_NONBLOCK);  

    //Key events init
    ioctl(fds, UI_SET_EVBIT, EV_KEY);
    for(int i=0; i<sizeof(keys)/sizeof(int); i++){
        ioctl(fds, UI_SET_KEYBIT, keys[i]);
    }
    
    //Mouse Pointer events init
    ioctl(fds, UI_SET_EVBIT, EV_REL);
    ioctl(fds, UI_SET_RELBIT, REL_X);
    ioctl(fds, UI_SET_RELBIT, REL_Y);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234; /* sample vendor */
    usetup.id.product = 0x5678; /* sample product */
    strcpy(usetup.name, "melih-hid");

    ioctl(fds, UI_DEV_SETUP, &usetup);
    ioctl(fds, UI_DEV_CREATE);
        
    //Create socket file descriptor
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket failed");
        exit(EXIT_FAILURE);   
    }

    //Attach socket to the port 8080
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //Bind
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
        perror("bind failed");
        exit(EXIT_FAILURE);   
    }

    if(listen(server_fd, 3)<0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

	fprintf(stderr, "Press 'q' to quit...\n");
	while (1) {
		ret = poll(pfds, 2, -1);
		if (ret < 0) {
			fprintf(stderr, "Cannot poll for fds: %m\n");
			break;
		}
		if (pfds[0].revents & POLLHUP) {
			fprintf(stderr, "Received HUP on stdin\n");
			break;
		}
		if (pfds[1].revents & POLLHUP) {
			fprintf(stderr, "Received HUP on uhid-cdev\n");
			break;
		}

		if (pfds[0].revents & POLLIN) {
			ret = keyboard(fd);
			if (ret)
				break;
		}
		if (pfds[1].revents & POLLIN) {
			ret = event(fd);
			if (ret)
				break;
		}

        if((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }

        char buffer[1024] = {0};
        valread = recv(new_socket, buffer, 1024, 0);
        int buffer_len = strlen(buffer);
        
        printf("Received: %s Size: %i\n", buffer, buffer_len); //For debugging

        short mode = buffer[0]-'0'; //char to int
        
        buffer[0] = '0'; //Making buffer '0' because atoi giving wrong value
        int incoming_code = atoi(buffer); // char* to int for mode 1 

        /*
         * Mode 2 / Mouse Pointer variables
        */
        int a = 0;
        int is_after_limiter = 0;
        char x_coor[6];
        char y_coor[6];

        /*
         * 1 - Keyboard
         * 2 - Mouse Pointer
         * 3 - Mouse Buttons
        */
        switch (mode){
            case 1: 
                emit(fd, EV_KEY, incoming_code, 1);
                emit(fd, EV_SYN, SYN_REPORT, 0);
                emit(fd, EV_KEY, incoming_code, 0);
                emit(fd, EV_SYN, SYN_REPORT, 0);
                usleep(5);
            break;
            case 2:
                //Splitting char* 2 parts by delimiter ":"           
                for(int i = 0; i < buffer_len; i++){
                    if(buffer[i] == ':'){
                        is_after_limiter = 1;
                    }
                    
                    if(is_after_limiter == 0){
                        *(x_coor+i) = buffer[i];
                    }else if(is_after_limiter == 1){
                        *(y_coor+a)= buffer[i+1];
                        a++;   
                    }else{
                        *(x_coor+i) = '\0';
                        *(y_coor+i) = '\0';
                    }           
                }
                
                emit(fd, EV_REL, REL_X, atoi(x_coor));
                emit(fd, EV_REL, REL_Y, atoi(y_coor));
                emit(fd, EV_SYN, SYN_REPORT, 0);
                usleep(5);
            break;
        }
        usleep(2);

	}
    sleep(1);

    //Detach HID Device
    ioctl(fd, UI_DEV_DESTROY);
    close(fd);

    //Close socket server
    close(server_fd);

	fprintf(stderr, "Destroy uhid device\n");
	destroy(fd);
	return EXIT_SUCCESS;
}