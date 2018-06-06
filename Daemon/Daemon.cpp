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
#define MAX_SIZE 1024

unsigned char vibrate[] = {0x00, 0x08, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00};
unsigned char vibrate_off[] = {0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct InputStruct
{
	unsigned char inputArray[20];
};

struct InputStruct* input;


unsigned char inputtemp[20];
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


int main()
{

	daemon(0, 0);

	int shm_fd = 0;

	struct mq_attr attr;
	char buffer[MAX_SIZE + 1];

	// initialize the queue attributes
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = MAX_SIZE;
	attr.mq_curmsgs = 0;

	// create the message queue
	
	mqd_t mq = mq_open("queue", O_CREAT | O_RDONLY, 0660, &attr);

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


	if (!startComms())
	{
		return -1;
	}

	bool shouldQuit = false;

	while (!shouldQuit)
	{
		ssize_t bytes_read;

		if (libusb_interrupt_transfer(h, ENDPOINTIN, inputtemp, sizeof(input), &transferred, NOTIMEOUT) == FAIL)
		{
			std::cout << "failed to send interupt transfer input" << std::endl;
			endComms();
			return -1;
		}

		vibrate[3] = inputtemp[4];
		vibrate[4] = inputtemp[5];

		if (libusb_interrupt_transfer(h, ENDPOINTOUT, vibrate, sizeof(vibrate), &transferred, NOTIMEOUT) == FAIL)
		{
			std::cout << "failed to send interupt transfer vibrate" << std::endl;
			endComms();
			return -1;
		}
		//std::cout << "voor sem";
		sem_wait(sem);
		memcpy(&input->inputArray, &inputtemp, sizeof(input->inputArray));
		std::cout << std::endl;
		sem_post(sem);
		bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
		if (bytes_read < 0)
		{
			perror("bytes read kleiner dan 0");
		}
		else
		{
			std::cout << bytes_read;
		}

		buffer[bytes_read] = 0;
		//std::cout << "Received" << buffer << std::endl;
	}
	endComms();
}
