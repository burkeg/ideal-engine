#include "worker.h"
#include <math.h>
#include "shmWrapper.h"
worker_data workerInfo;

void work(int workerID) {
  printf("Entering worker loop\n");
  sleep(1);
  key_t shmkey;
  int shmid;
  unsigned char *raw_data;
  unsigned char *raw_finished;
  char *workers_not_empty_str = "wksActive";
  sem_t *workers_not_empty;
  sem_t *sem_start;
  char *worker_tmp_name;
  int i,sum;
  char filecontent[30];
  

  raw_data= (unsigned char *) shmat (attachShm(workerID), NULL, 0);
  //visualize mapped memory
  //for (int i = 0; i < 4; i++) {
  // printf("%d,",((int*) raw_data)[i]);
  //}
  //printf("\n");
  
  //link ptrs to shared memory
  workerInfo.worker_type=((int*) raw_data);
  //  workerInfo.status=&(((int*) raw_data)[1]);
  workerInfo.task_index=&(((int*) raw_data)[2]);


  //map worker finished status bits
  raw_finished= (unsigned char *) shmat (attachShm(NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+NUM_WORKERS+2), NULL, 0);
  workerInfo.finished=(int*) raw_finished;

  //open semaphores
  workers_not_empty=open_existing_sem(workers_not_empty_str);
  
  worker_tmp_name=malloc(sizeof(char)*255);
  sprintf(worker_tmp_name,"worker%03d",workerID-1);
  sem_start = open_existing_sem(worker_tmp_name);
  free(worker_tmp_name);

  srand(0);
  while (1) {
    //wait on start signal
    //printf(":sem_start wait %d\n",workerID);
    sem_wait(sem_start);
    if (workerInfo.finished[NUM_WORKERS]==1)
      break;
    //printf(":sem_start finished wait %d\n",workerID);

    //critical section

    //FILE *fp;
    //char fname[15];
    //sprintf(fname,"mapOut%d.txt",*(workerInfo.task_index));
    //fp = fopen(fname,"w");
    //sprintf(filecontent,"%d\n\0",fib(NUM_PARTITIONS_MAP-1-*(workerInfo.task_index)));
    //fwrite(filecontent,1,sizeof(filecontent),fp);
    //printf(filecontent);
    //fclose(fp);
    //*
    sum=0;
    for (i=0; i < 1000000; i++) {
      sum += sin((double)i);
    }//*/
    /*
    while (1) {
      printf("fail %d\n",workerID);
      if (rand() % 10 == 0){
	break;
      }
    }*/
    //printf("                       %d\n",sum);
    //printf("--I AM DOING WORK NOW %d\n",workerID);
    
    //workerInfo.finished[workerID-1]=*(workerInfo.task_index);
    workerInfo.finished[workerID-1]=1;
    
    //printf(":%s posted %d\n",workers_not_empty_str,workerID);
    printProgress();
    sem_post(workers_not_empty);
    //signal finished
  }

  printf("Map phase complete: worker %d\n",workerID);
  //detach from worker shared memory
  detachShm(workerID,raw_data);
  detachShm(NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+NUM_WORKERS+2,raw_finished);
  printf("Worker %d detached from Shm\n",workerID);
}

void printProgress() {
  char str[NUM_WORKERS+2];
  int i;
  for (i = 0; i < NUM_WORKERS; i++) {
    str[NUM_WORKERS-1-i]=workerInfo.finished[i]? '1' : '0';
  }
  str[NUM_WORKERS]='\n';
  str[NUM_WORKERS+1]='\0';
  printf("worker status: %s",str);
}

int fib(int n){
  return n<=2 ? 1 : fib(n-1)+fib(n-2);
}
