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


struct InputStruct
{
	unsigned char inputarray[20];
};


int main()
{
	std::cout << "Test begin Main" << std::endl;

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
		std::cout << "Daemon to Server Failed" << std::endl;
	}
	int reading = 1;
	std::cout << "hoi" << std::endl;
	while (reading)
	{
		sem_wait(sem);
		for (unsigned int i = 0; i < sizeof(input->inputarray); i++)
		{
			std::cout << (int)input->inputarray[i] << "--";
		}
		std::cout << std::endl;
		sem_post(sem);
	}
}
