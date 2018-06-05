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

#define nameSize 80


struct InputStruct
{
  unsigned char inputarray[20];
};


//#define SHM_SIZE sizeof(struct InputStruct)

/*void handleInput(int choice)
{
  char queue_name[nameSize] = "MessageQueue";

  int returnvalue = -1;

  mqd_t mq_fd = -1;
  mq_fd = mq_open(queue_name,O_WRONLY);

  if (mq_fd != -1)
  {
    returnvalue = mq_send(mq_fd, (char*) &choice, sizeof(int), 0);
    if (returnvalue == -1)
     {
      printf("Error sending message to queue.");
    }
    mq_unlink(queue_name);
  }
}
*/
int main()
{
  struct InputStruct* input;
  int shm_fd = 0;
  int position = 0;

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
  while (reading)
  {
    sem_wait(sem);
    for (unsigned int i = 0; i < sizeof(input->inputarray); i++)
    {
      std::cout << input->inputarray[i] << std::endl;
    }

    sem_post(sem);
    position++;
  }

}
