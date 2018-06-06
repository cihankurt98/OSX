#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <iostream>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <string.h>


struct InputStruct
{
	unsigned char inputarray[20];
};

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


int main()
{
	char *data;
	printf("%s%c%c\n",
	       "Content-Type:text/html;charset=iso-8859-1", 13, 10);
	printf("<TITLE>CGI</TITLE>\n");
	printf("<H3>CGI</H3>\n");
	printf("<H4>Klik op een button om de waardes te zien.</H4>\n");
	printf("<meta http-equiv= \"refresh\" content=\"1\">");
	data = getenv("QUERY_STRING");
	if (data == NULL)
		printf("<P>No message found </P>");

	struct InputStruct* input;
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

	if ((input = (struct InputStruct*) mmap(0, sizeof(struct InputStruct), PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED)
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
		std::cout << "Daemon to Server Failed";
	}

	sem_post(sem);
	if (input->inputarray[3] & BUTTON_A_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_one, sizeof(led_one), &transferred2, NOTIMEOUT);
		std::cout << "Button A: " << "Pressed<br>";
	}

	if (input->inputarray[3] & BUTTON_B_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_two, sizeof(led_two), &transferred2, NOTIMEOUT);
		std::cout << "Button B: " << "Pressed<br>";
	}

	if (input->inputarray[3] & BUTTON_X_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_three, sizeof(led_three), &transferred2, NOTIMEOUT);
		std::cout << "Button X: " << "Pressed<br>";
	}

	if (input->inputarray[3] & BUTTON_Y_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_four, sizeof(led_four), &transferred2, NOTIMEOUT);
		std::cout << "Button Y: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_B_OFFSET && input->inputarray[0] == 0)
	{
		std::cout << "Button Back: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_A_OFFSET && input->inputarray[0] == 0)
	{
		std::cout << "Button Start: " << "Pressed<br>";
	}

	if (input->inputarray[3] & BUTTON_XBOX_OFFSET && input->inputarray[0] == 0)
	{
		std::cout << "Button Xbox: " << "Triggered<br>";
	}

	if (input->inputarray[3] & BUTTON_TRIGGER_R1_OFFSET && input->inputarray[0] == 0 )
	{
		std::cout << "Trigger R1: " << "Triggered<br>";
	}

	if (input->inputarray[3] & BUTTON_TRIGGER_L1_OFFSET && input->inputarray[0] == 0)
	{
		std::cout << "Trigger L1: " << "Triggered<br>";
	}

	if (input->inputarray[2] & BUTTON_LEFT_JOYSTICK_OFFSET && input->inputarray[0] == 0)
	{
		std::cout << "Left joystick: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_RIGHT_JOYSTICK_OFFSET && input->inputarray[0] == 0)
	{
		std::cout << "Right joystick: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_DPAD_UP_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_rotating, sizeof(led_rotating), &transferred, NOTIMEOUT);
		std::cout << "D-Pad Up: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_DPAD_DOWN_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_blinking, sizeof(led_blinking), &transferred, NOTIMEOUT);
		std::cout << "D-Pad Down: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_DPAD_LEFT_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_alternating, sizeof(led_alternating), &transferred, NOTIMEOUT);
		std::cout << "D-Pad Left: " << "Pressed<br>";
	}

	if (input->inputarray[2] & BUTTON_DPAD_RIGHT_OFFSET && input->inputarray[0] == 0)
	{
		//libusb_interrupt_transfer(h, ENDPOINTOUT, led_slowblinking, sizeof(led_slowblinking), &transferred, NOTIMEOUT);
		std::cout << "D-Pad Right: " << "Pressed<br>";
	}

	sem_wait(sem);
}
