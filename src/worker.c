#include "worker.h"

void work(int workerID) {
  printf("Entering worker loop\n");
  sleep(5);
  worker_data workerInfo;
  key_t shmkey;
  int shmid;
  unsigned char *raw_data;
  char *workers_not_empty_str = "wksActive";
  sem_t *workers_not_empty;
  sem_t *sem_start;
  char *worker_tmp_name;
  
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
  //  workerInfo.status=&(((int*) raw_data)[1]);
  workerInfo.task_index=&(((int*) raw_data)[2]);
  
  //open semaphores
  workers_not_empty=open_existing_sem(workers_not_empty_str);
  
  worker_tmp_name=malloc(sizeof(char)*255);
  sprintf(worker_tmp_name,"worker%03d",workerID);
  sem_start = open_existing_sem(worker_tmp_name);
  free(worker_tmp_name);
  
  
  while (1) {
    //wait on start signal
    printf(":sem_start wait\n");
    sem_wait(sem_start);
    printf(":sem_start finished wait\n");
    

    printf("I AM DOING WORK NOW\n");
    
    //workerInfo.finished[workerID]=*(workerInfo.task_index);
    workerInfo.finished[workerID]=1;
    sem_post(workers_not_empty);
    //signal finished
  }

  //detach from worker shared memory
  shmdt(raw_data);
}
