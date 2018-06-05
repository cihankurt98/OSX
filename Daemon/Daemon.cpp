#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>
#include <iostream>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <string.h>

#define LED1_ON 0x06
#define LED2_ON 0x07
#define LED3_ON 0x08
#define LED4_ON 0x09

#define BUTTON_A_OFFSET 0x10
#define BUTTON_B_OFFSET 0x20
#define BUTTON_X_OFFSET 0x40
#define BUTTON_Y_OFFSET 0x80
#define BUTTON_XBOX_OFFSET 0x04
#define BUTTON_TRIGGER_R1_OFFSET 0x02
#define BUTTON_TRIGGER_L1_OFFSET 0x01
#define BUTTON_LEFT_JOYSTICK_OFFSET 0x40
#define BUTTON_RIGHT_JOYSTICK_OFFSET 0x80
#define BUTTON_DPAD_UP_OFFSET 0X01
#define BUTTON_DPAD_DOWN_OFFSET 0X02
#define BUTTON_DPAD_LEFT_OFFSET 0X04
#define BUTTON_DPAD_RIGHT_OFFSET 0X08

//#define SHM_SIZE sizeof(command)
#define ENDPOINTOUT 0x01
#define ENDPOINTIN 0x81

#define NOTIMEOUT 0
#define FAIL 1

unsigned char vibrate[] = {0x00, 0x08, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00};
unsigned char vibrate_off[] = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

unsigned char led_one[] = { 0x01, 0x03, LED1_ON}; //message type, length, ledvalue
unsigned char led_two[] = { 0x01, 0x03, LED2_ON};
unsigned char led_three[] = { 0x01, 0x03, LED3_ON};
unsigned char led_four[] = { 0x01, 0x03, LED4_ON};


struct InputStruct
{
	unsigned char inputArray[20];
};





int transferred;
uint16_t vid = 0x045e;
uint16_t pid = 0x028e;

libusb_device_handle* h;


void endComms()
{
	if (libusb_interrupt_transfer(h, ENDPOINTOUT, vibrate_off, sizeof(vibrate_off), &transferred, NOTIMEOUT) == FAIL)
	{
		std::cout << "failed to send interupt tranfer vibrate" << std::endl;
	}

	libusb_close(h);
}

bool startComms()
{
	if (libusb_init(NULL)  == FAIL)
	{
		std::cout << "init failed." << std::endl;
		return false;
	}

	h = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (h == NULL)
	{
		std::cout << "failed to open device" << std::endl;
		return false;
	}
	return true;
}

void write()
{
	memcpy(&input->inputarray, &input, sizeof(struct InputStruct));
}

int main()
{
	struct InputStruct* input;
	daemon(0, 0);

	int shm_fd = 0;

	if ((shm_fd = shm_open ("sharedmem", O_CREAT | O_RDWR, 0666)) == -1)
	{
		perror("Can not open the shared memory \n");
		return 0;
	}

	if (ftruncate(shm_fd, sizeof(struct InputStruct)) != 0)
	{
		perror("Can not set the size of the shared memory \n");
		return 0;
	}

	if ((input = (struct InputArray*) mmap(0, sizeof(struct InputStruct), PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
	{
		perror("Can not map");
		return -1;
	}

	if (mlock(input, sizeof(struct InputStruct)) != 0)
	{
		perror("cannot mlock");
		return -1;
	}

	sem_t* sem = sem_open("DaemonToServer", O_CREAT, 0666, 0);
	if (sem == SEM_FAILED)
	{
		std::cout << "Daemon to Server Failed" << std::endl;
	}


	if (!startComms())
	{
		return -1;
	}

	bool shouldQuit = false;

	while (!shouldQuit)
	{


		if (libusb_interrupt_transfer(h, ENDPOINTIN, input, sizeof(input), &transferred, NOTIMEOUT) == FAIL)
		{
			std::cout << "failed to send interupt transfer input" << std::endl;
			endComms();
			return -1;
		}

		vibrate[3] = input[4];
		vibrate[4] = input[5];

		if (libusb_interrupt_transfer(h, ENDPOINTOUT, vibrate, sizeof(vibrate), &transferred, NOTIMEOUT) == FAIL)
		{
			std::cout << "failed to send interupt transfer vibrate" << std::endl;
			endComms();
			return -1;
		}


		for (unsigned int i = 0; i < sizeof(input); i++)
		{
			std::cout << (int)input[i] << "-";
		}
		std::cout << std::endl;

		if (inputArray.input[3] & BUTTON_A_OFFSET && inputArray.input[0] == 0)
		{
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_one, sizeof(led_one), &transferred2, NOTIMEOUT)
			write();
		}

		if (inputArray.input[3] & BUTTON_B_OFFSET && inputArray.input[0] == 0)
		{
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_two, sizeof(led_two), &transferred2, NOTIMEOUT);
			write();
		}

		if (inputArray.input[3] & BUTTON_X_OFFSET && inputArray.input[0] == 0)
		{
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_three, sizeof(led_three), &transferred2, NOTIMEOUT);
			write();
		}

		if (inputArray.input[3] & BUTTON_Y_OFFSET && inputArray.input[0] == 0)
		{
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_four, sizeof(led_four), &transferred2, NOTIMEOUT);
			write();
		}

		if (inputArray.input[2] & BUTTON_B_OFFSET && inputArray.input[0] == 0)
		{
			write();
		}

		if (inputArray.input[2] & BUTTON_A_OFFSET && inputArray.input[0] == 0)
		{
			write();
		}

		if (inputArray.input[3] & BUTTON_XBOX_OFFSET && inputArray.input[0] == 0)
		{
			write();
		}
		if (inputArray.input[5] > 0 && inputArray.input[0] == 0)
		{
			//std::cout << "Trigger R2: " << "Triggered" << std::endl;
			write();
		}

		if (inputArray.input[3] & BUTTON_TRIGGER_R1_OFFSET && inputArray.input[0] == 0 )
		{
			//std::cout << "Trigger R1: " << "Triggered" << std::endl;
			write();
		}

		if (inputArray.input[4] > 0 && inputArray.input[0] == 0)
		{
			//std::cout << "Trigger L2: " << "Triggered" << std::endl;
			write();
		}

		if (inputArray.input[3] & BUTTON_TRIGGER_L1_OFFSET && inputArray.input[0] == 0)
		{
			//std::cout << "Trigger L1: " << "Triggered" << std::endl;
			write();
		}

		if (inputArray.input[2] & BUTTON_LEFT_JOYSTICK_OFFSET && inputArray.input[0] == 0)
		{
			write();
			std::cout << "Left joystick: " << "Pressed" << std::endl;
		}

		if (inputArray.input[2] & BUTTON_RIGHT_JOYSTICK_OFFSET && inputArray.input[0] == 0)
		{
			write();
			std::cout << "Right joystick: " << "Pressed" << std::endl;
		}

		std::cout << "Left joystick X axis: " << (int) (inputArray.input[6]) << std::endl;
		std::cout << "Left joystick Y axis: " << (int) (inputArray.input[8]) << std::endl;
		std::cout << "Right joystick X axis: " << (int) (inputArray.input[10]) << std::endl;
		std::cout << "Right joystick Y axis: " << (int) (inputArray.input[12]) << std::endl;

		if (inputArray.input[2] & BUTTON_DPAD_UP_OFFSET && inputArray.input[0] == 0)
		{
			write();
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_rotating, sizeof(led_rotating), &transferred, NOTIMEOUT);
			//std::cout << "D-Pad Up: " << "Pressed" << std::endl;
		}

		if (inputArray.input[2] & BUTTON_DPAD_DOWN_OFFSET && inputArray.input[0] == 0)
		{
			write();
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_blinking, sizeof(led_blinking), &transferred, NOTIMEOUT);
			//std::cout << "D-Pad Down: " << "Pressed" << std::endl;
		}

		if (inputArray.input[2] & BUTTON_DPAD_LEFT_OFFSET && inputArray.input[0] == 0)
		{
			write();
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_alternating, sizeof(led_alternating), &transferred, NOTIMEOUT);
			//std::cout << "D-Pad Left: " << "Pressed" << std::endl;
		}

		if (inputArray.input[2] & BUTTON_DPAD_RIGHT_OFFSET && inputArray.input[0] == 0)
		{
			write();
			//libusb_interrupt_transfer(h, ENDPOINTOUT, led_slowblinking, sizeof(led_slowblinking), &transferred, NOTIMEOUT);
			//std::cout << "D-Pad Right: " << "Pressed" << std::endl;
		}
	}
	endComms();
}
