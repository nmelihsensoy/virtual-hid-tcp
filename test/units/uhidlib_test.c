// SPDX-License-Identifier: GPL-2.0
/*
 * UHID Example
 *
 * Copyright (c) 2012-2013 David Herrmann <dh.herrmann@gmail.com>
 *
 * The code may be used by anyone for any purpose,
 * and can serve as a starting point for developing
 * applications using uhid.
 */

/*
 * UHID Example
 * This example emulates a basic 3 buttons mouse with wheel over UHID. Run this
 * program as root and then use the following keys to control the mouse:
 *   q: Quit the application
 *   1: Toggle left button (down, up, ...)
 *   2: Toggle right button
 *   3: Toggle middle button
 *   a: Move mouse left
 *   d: Move mouse right
 *   w: Move mouse up
 *   s: Move mouse down
 *   r: Move wheel up
 *   f: Move wheel down
 *
 * Additionally to 3 button mouse, 3 keyboard LEDs are also supported (LED_NUML,
 * LED_CAPSL and LED_SCROLLL). The device doesn't generate any related keyboard
 * events, though. You need to manually write the EV_LED/LED_XY/1 activation
 * input event to the evdev device to see it being sent to this device.
 *
 * If uhid is not available as /dev/uhid, then you can pass a different path as
 * first argument.
 * If <linux/uhid.h> is not installed in /usr, then compile this with:
 *   gcc -o ./uhid_test -Wall -I./include ./samples/uhid/uhid-example.c
 * And ignore the warning about kernel headers. However, it is recommended to
 * use the installed uhid.h if available.
 */

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <linux/uhid.h>
#include <sys/types.h>
#include <sys/stat.h>
/*
 * HID Report Desciptor
 * We emulate a basic 3 button mouse with wheel and 3 keyboard LEDs. This is
 * the report-descriptor as the kernel will parse it:
 *
 * INPUT(1)[INPUT]
 *   Field(0)
 *     Physical(GenericDesktop.Pointer)
 *     Application(GenericDesktop.Mouse)
 *     Usage(3)
 *       Button.0001
 *       Button.0002
 *       Button.0003
 *     Logical Minimum(0)
 *     Logical Maximum(1)
 *     Report Size(1)
 *     Report Count(3)
 *     Report Offset(0)
 *     Flags( Variable Absolute )
 *   Field(1)
 *     Physical(GenericDesktop.Pointer)
 *     Application(GenericDesktop.Mouse)
 *     Usage(3)
 *       GenericDesktop.X
 *       GenericDesktop.Y
 *       GenericDesktop.Wheel
 *     Logical Minimum(-128)
 *     Logical Maximum(127)
 *     Report Size(8)
 *     Report Count(3)
 *     Report Offset(8)
 *     Flags( Variable Relative )
 * OUTPUT(2)[OUTPUT]
 *   Field(0)
 *     Application(GenericDesktop.Keyboard)
 *     Usage(3)
 *       LED.NumLock
 *       LED.CapsLock
 *       LED.ScrollLock
 *     Logical Minimum(0)
 *     Logical Maximum(1)
 *     Report Size(1)
 *     Report Count(3)
 *     Report Offset(0)
 *     Flags( Variable Absolute )
 *
 * This is the mapping that we expect:
 *   Button.0001 ---> Key.LeftBtn
 *   Button.0002 ---> Key.RightBtn
 *   Button.0003 ---> Key.MiddleBtn
 *   GenericDesktop.X ---> Relative.X
 *   GenericDesktop.Y ---> Relative.Y
 *   GenericDesktop.Wheel ---> Relative.Wheel
 *   LED.NumLock ---> LED.NumLock
 *   LED.CapsLock ---> LED.CapsLock
 *   LED.ScrollLock ---> LED.ScrollLock
 *
 * This information can be verified by reading /sys/kernel/debug/hid/<dev>/rdesc
 * This file should print the same information as showed above.
 */

static unsigned char rdescOLD[] = {
	0x05, 0x01,	/* USAGE_PAGE (Generic Desktop) */
	0x09, 0x02,	/* USAGE (Mouse) */
	0xa1, 0x01,	/* COLLECTION (Application) */
	0x09, 0x01,		/* USAGE (Pointer) */
	0xa1, 0x00,		/* COLLECTION (Physical) */
	0x85, 0x01,			/* REPORT_ID (1) */
	0x05, 0x09,			/* USAGE_PAGE (Button) */
	0x19, 0x01,			/* USAGE_MINIMUM (Button 1) */
	0x29, 0x03,			/* USAGE_MAXIMUM (Button 3) */
	0x15, 0x00,			/* LOGICAL_MINIMUM (0) */
	0x25, 0x01,			/* LOGICAL_MAXIMUM (1) */
	0x95, 0x03,			/* REPORT_COUNT (3) */
	0x75, 0x01,			/* REPORT_SIZE (1) */
	0x81, 0x02,			/* INPUT (Data,Var,Abs) */
	0x95, 0x01,			/* REPORT_COUNT (1) */
	0x75, 0x05,			/* REPORT_SIZE (5) */
	0x81, 0x01,			/* INPUT (Cnst,Var,Abs) */
	0x05, 0x01,			/* USAGE_PAGE (Generic Desktop) */
	0x09, 0x30,			/* USAGE (X) */
	0x09, 0x31,			/* USAGE (Y) */
	0x09, 0x38,			/* USAGE (WHEEL) */
	0x15, 0x81,			/* LOGICAL_MINIMUM (-127) */
	0x25, 0x7f,			/* LOGICAL_MAXIMUM (127) */
	0x75, 0x08,			/* REPORT_SIZE (8) */
	0x95, 0x03,			/* REPORT_COUNT (3) */
	0x81, 0x06,			/* INPUT (Data,Var,Rel) */
	0xc0,			/* END_COLLECTION */
	0xc0,		/* END_COLLECTION */
	0x05, 0x01,	/* USAGE_PAGE (Generic Desktop) */
	0x09, 0x06,	/* USAGE (Keyboard) */
	0xa1, 0x01,	/* COLLECTION (Application) */
	0x85, 0x02,		/* REPORT_ID (2) */
	0x05, 0x08,		/* USAGE_PAGE (Led) */
	0x19, 0x01,		/* USAGE_MINIMUM (1) */
	0x29, 0x03,		/* USAGE_MAXIMUM (3) */
	0x15, 0x00,		/* LOGICAL_MINIMUM (0) */
	0x25, 0x01,		/* LOGICAL_MAXIMUM (1) */
	0x95, 0x03,		/* REPORT_COUNT (3) */
	0x75, 0x01,		/* REPORT_SIZE (1) */
	0x91, 0x02,		/* Output (Data,Var,Abs) */
	0x95, 0x01,		/* REPORT_COUNT (1) */
	0x75, 0x05,		/* REPORT_SIZE (5) */
	0x91, 0x01,		/* Output (Cnst,Var,Abs) */
	0xc0,		/* END_COLLECTION */
};

static unsigned char rdescLOGI[] = {
	0x05, 0x01,                    // Usage Page (Generic Desktop)        0
	0x09, 0x06,                    // Usage (Keyboard)                    2
	0xa1, 0x01,                    // Collection (Application)            4
	0x85, 0x01,                    //  Report ID (1)                      6
	0x05, 0x07,                    //  Usage Page (Keyboard)              8
	0x19, 0xe0,                    //  Usage Minimum (224)                10
	0x29, 0xe7,                    //  Usage Maximum (231)                12
	0x15, 0x00,                    //  Logical Minimum (0)                14
	0x25, 0x01,                    //  Logical Maximum (1)                16
	0x75, 0x01,                    //  Report Size (1)                    18
	0x95, 0x08,                    //  Report Count (8)                   20
	0x81, 0x02,                    //  Input (Data,Var,Abs)               22
	0x81, 0x03,                    //  Input (Cnst,Var,Abs)               24
	0x95, 0x06,                    //  Report Count (6)                   26
	0x75, 0x08,                    //  Report Size (8)                    28
	0x15, 0x00,                    //  Logical Minimum (0)                30
	0x26, 0xa4, 0x00,              //  Logical Maximum (164)              32
	0x19, 0x00,                    //  Usage Minimum (0)                  35
	0x2a, 0xa4, 0x00,              //  Usage Maximum (164)                37
	0x81, 0x00,                    //  Input (Data,Arr,Abs)               40
	0xc0,                          // End Collection                      42
	0x05, 0x0c,                    // Usage Page (Consumer Devices)       43
	0x09, 0x01,                    // Usage (Consumer Control)            45
	0xa1, 0x01,                    // Collection (Application)            47
	0x85, 0x03,                    //  Report ID (3)                      49
	0x75, 0x10,                    //  Report Size (16)                   51
	0x95, 0x02,                    //  Report Count (2)                   53
	0x15, 0x01,                    //  Logical Minimum (1)                55
	0x26, 0x8c, 0x02,              //  Logical Maximum (652)              57
	0x19, 0x01,                    //  Usage Minimum (1)                  60
	0x2a, 0x8c, 0x02,              //  Usage Maximum (652)                62
	0x81, 0x00,                    //  Input (Data,Arr,Abs)               65
	0xc0,                          // End Collection                      67
	0x05, 0x01,                    // Usage Page (Generic Desktop)        68
	0x09, 0x80,                    // Usage (System Control)              70
	0xa1, 0x01,                    // Collection (Application)            72
	0x85, 0x04,                    //  Report ID (4)                      74
	0x75, 0x02,                    //  Report Size (2)                    76
	0x95, 0x01,                    //  Report Count (1)                   78
	0x15, 0x01,                    //  Logical Minimum (1)                80
	0x25, 0x03,                    //  Logical Maximum (3)                82
	0x09, 0x82,                    //  Usage (System Sleep)               84
	0x09, 0x81,                    //  Usage (System Power Down)          86
	0x09, 0x83,                    //  Usage (System Wake Up)             88
	0x81, 0x60,                    //  Input (Data,Arr,Abs,NoPref,Null)   90
	0x75, 0x06,                    //  Report Size (6)                    92
	0x81, 0x03,                    //  Input (Cnst,Var,Abs)               94
	0xc0,                          // End Collection                      96
	0x06, 0x00, 0xff,              // Usage Page (Vendor Defined Page 1)  97
	0x09, 0x01,                    // Usage (Vendor Usage 1)              100
	0xa1, 0x01,                    // Collection (Application)            102
	0x85, 0x10,                    //  Report ID (16)                     104
	0x75, 0x08,                    //  Report Size (8)                    106
	0x95, 0x06,                    //  Report Count (6)                   108
	0x15, 0x00,                    //  Logical Minimum (0)                110
	0x26, 0xff, 0x00,              //  Logical Maximum (255)              112
	0x09, 0x01,                    //  Usage (Vendor Usage 1)             115
	0x81, 0x00,                    //  Input (Data,Arr,Abs)               117
	0x09, 0x01,                    //  Usage (Vendor Usage 1)             119
	0x91, 0x00,                    //  Output (Data,Arr,Abs)              121
	0xc0,                          // End Collection                      123
	0x06, 0x00, 0xff,              // Usage Page (Vendor Defined Page 1)  124
	0x09, 0x02,                    // Usage (Vendor Usage 2)              127
	0xa1, 0x01,                    // Collection (Application)            129
	0x85, 0x11,                    //  Report ID (17)                     131
	0x75, 0x08,                    //  Report Size (8)                    133
	0x95, 0x13,                    //  Report Count (19)                  135
	0x15, 0x00,                    //  Logical Minimum (0)                137
	0x26, 0xff, 0x00,              //  Logical Maximum (255)              139
	0x09, 0x02,                    //  Usage (Vendor Usage 2)             142
	0x81, 0x00,                    //  Input (Data,Arr,Abs)               144
	0x09, 0x02,                    //  Usage (Vendor Usage 2)             146
	0x91, 0x00,                    //  Output (Data,Arr,Abs)              148
	0xc0,                          // End Collection                      150
};

static unsigned char rdesc[] = {
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
	0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x95, 0x02,                    //     REPORT_COUNT (2)
	0x81, 0x06,                    //     INPUT (Data,Var,Rel)

	0x09, 0x38,			/* USAGE (WHEEL) */
	0x15, 0x81,			/* LOGICAL_MINIMUM (-127) */
	0x25, 0x7f,			/* LOGICAL_MAXIMUM (127) */
	0x75, 0x08,			/* REPORT_SIZE (8) */
	0x95, 0x03,			/* REPORT_COUNT (3) */
	0x81, 0x06,			/* INPUT (Data,Var,Rel) */
	0xc0,                          //   END_COLLECTION
	0xc0                           // END_COLLECTION
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

int main(int argc, char **argv)
{
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
	}

	fprintf(stderr, "Destroy uhid device\n");
	destroy(fd);
	return EXIT_SUCCESS;
}