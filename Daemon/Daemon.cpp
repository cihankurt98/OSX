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
#include <mqueue.h>

#define ENDPOINTOUT 0x01
#define ENDPOINTIN 0x81
#define BUTTON_A_OFFSET 0x10
#define NOTIMEOUT 0
#define FAIL 1

#define LED1_ON 0x06
#define LED2_ON 0x07
#define LED3_ON 0x08
#define LED4_ON 0x09
#define LEDS_OFF 0x00

#define VIBRATE_ON "VIBRATE_ON"
#define VIBRATE_OFF "VIBRATE_OFF"
#define LED_1 "LED_1_ON"
#define LED_2 "LED_2_ON"
#define LED_3 "LED_3_ON"
#define LED_4 "LED_4_ON"
#define EXIT "EXIT"

unsigned char vibrate_on[] = {0x00, 0x08, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00};
unsigned char vibrate_off[] = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char led_one[] = { 0x01, 0x03, LED1_ON}; //message type, length, ledvalue
unsigned char led_two[] = { 0x01, 0x03, LED2_ON};
unsigned char led_three[] = { 0x01, 0x03, LED3_ON};
unsigned char led_four[] = { 0x01, 0x03, LED4_ON};


struct InputStruct
{
	unsigned char inputArray[20];
};

struct MessageQueue
{
	char data[1024];
};

struct InputStruct* input;


unsigned char inputtemp[20];
int transferred;
uint16_t vid = 0x045e;
uint16_t pid = 0x028e;

libusb_device_handle* device;

mqd_t fd;

void ChangeVibrateState(u_char* state)
{
	if (libusb_interrupt_transfer(device, ENDPOINTOUT, state, sizeof(state)
	                              , &transferred, NOTIMEOUT) != 0)
	{
		perror("ChangeVibrateState interrupt transfer");
	}
}

void ChangeLedState(int state)
{
	unsigned char temparray[2] = state;
	if (libusb_interrupt_transfer(device, ENDPOINTOUT, temparray, sizeof(temparray)
		, &transferred, NOTIMEOUT))
	{
		perror("ChangeLedState interrupt transfer");
	}
}

void MqRcv(MessageQueue* buff)
{
	int bytes;
	bytes = mq_receive(fd, (char*)(buff), sizeof(MessageQueue), NULL);
	if ( bytes < 0)
	{
		perror("Receive failed MqRCV");
	}
	buff->data[bytes] = 0; //null terminator

}

void* ReadMQ(void* threadArgs)
{
	int stop = 1;
	if (pthread_detach (pthread_self()) != 0)
	{
		perror("Detach thread failed");
	}

	MessageQueue* Queue = new MessageQueue();
	while (stop)
	{
		MqRcv(Queue);

		if (strcmp(Queue->data, VIBRATE_ON) == 0) ChangeVibrateState(vibrate_on);
		if (strcmp(Queue->data, VIBRATE_OFF) == 0) ChangeVibrateState(vibrate_off);
		if (strcmp(Queue->data, LED_1) == 0) ChangeLedState(LED1_ON);
		if (strcmp(Queue->data, LED_2) == 0) ChangeLedState(LED2_ON);
		if (strcmp(Queue->data, LED_3) == 0) ChangeLedState(LED3_ON);
		if (strcmp(Queue->data, LED_4) == 0) ChangeLedState(LED4_ON);

		if (strcmp(Queue->data, EXIT) == 0)
		{
			ChangeVibrateState(vibrate_off);
			ChangeLedState(LEDS_OFF);
			stop = 0;
		}
	}
	std::cout << "Exiting thread" << std::endl;
	return NULL;
}


void endComms()
{
	if (libusb_interrupt_transfer(device, ENDPOINTOUT, vibrate_off, sizeof(vibrate_off), &transferred, NOTIMEOUT) == FAIL)
	{
		std::cout << "failed to send interupt tranfer vibrate" << std::endl;
	}

	libusb_close(device);
}

bool startComms()
{
	if (libusb_init(NULL)  == FAIL)
	{
		std::cout << "init failed." << std::endl;
		return false;
	}

	device = libusb_open_device_with_vid_pid(NULL, vid, pid);
	if (device == NULL)
	{
		std::cout << "failed to open device" << std::endl;
		return false;
	}
	return true;
}
int main()
{
	fd = mq_open("MessQueue", O_CREAT | O_RDONLY, 0666, &attr);

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
		std::cout << "Daemon to Server Failed" << std::endl;
	}

	pthread_t ID;
	if (pthread_create(&ID, NULL, ReadMQ, (void*) NULL) < 0)
	{
		perror("Creating a thread failed");
	}


	if (!startComms())
	{
		return -1;
	}

	bool shouldQuit = false;

	while (!shouldQuit)
	{

		if (libusb_interrupt_transfer(device, ENDPOINTIN, inputtemp, sizeof(input), &transferred, NOTIMEOUT) == FAIL)
		{
			std::cout << "failed to send interupt transfer input" << std::endl;
			endComms();
			return -1;
		}

		vibrate[3] = inputtemp[4];
		vibrate[4] = inputtemp[5];

		if (libusb_interrupt_transfer(device, ENDPOINTOUT, vibrate, sizeof(vibrate), &transferred, NOTIMEOUT) == FAIL)
		{
			std::cout << "failed to send interupt transfer vibrate" << std::endl;
			endComms();
			return -1;
		}

		sem_wait(sem);
		memcpy(&input->inputArray, &inputtemp, sizeof(input->inputArray));
		std::cout << std::endl;
		sem_post(sem);
	}
	endComms();
}
