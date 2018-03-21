#include "worker.h"

void work(int workerID) {
  printf("Entering worker loop\n");
  worker_data workerInfo;
  key_t shmkey;
  int shmid;
  unsigned char *raw_data;
  
  //map worker data and string data to shared buffer
  shmkey=ftok(exe_name,workerID);
  shmid = shmget (shmkey, sizeof (int)*WORKER_SHM_SIZE, 0644);
  if (shmid < 0){ //shared memory error check
    perror ("shmget\n");
  }

  printf("Shared memory opened in worker (ID,shmid,shmkey:%d,%d,%d\n",workerID,shmid,shmkey);

  raw_data= (char *) shmat (shmid, NULL, 0);
  //visualize mapped memory
  //for (int i = 0; i < 4; i++) {
  // printf("%d,",((int*) raw_data)[i]);
  //}
  //printf("\n");
  
  
  //link ptrs to shared memory
  workerInfo.worker_type=((int*) raw_data);
  workerInfo.status=&(((int*) raw_data)[1]);
  workerInfo.task_index=&(((int*) raw_data)[2]);
  
  //open semaphores
  
  
  while (1) {
    //wait on start signal
    break;



    //signal finished
  }

  //detach from worker shared memory
  shmdt(raw_data);
}
