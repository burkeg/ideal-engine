#include "worker.h"
#include <math.h>
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
  
  //map worker data and string data to shared buffer
  shmkey=ftok(exe_name,workerID);
  shmid = shmget (shmkey, sizeof (int)*WORKER_SHM_SIZE, 0644);
  if (shmid < 0){ //shared memory error check
    perror ("shmget\n");
  }

  printf("Shared memory opened in worker (ID,shmid,shmkey:%d,%d,%d\n",workerID,shmid,shmkey);

  raw_data= (unsigned char *) shmat (shmid, NULL, 0);
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
  shmkey=ftok(exe_name,NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+NUM_WORKERS+2);
  shmid = shmget (shmkey, sizeof (int)*NUM_WORKERS, 0644);
  if (shmid < 0){ //shared memory error check
    perror ("shmget\n");
  }

  raw_finished= (unsigned char *) shmat (shmid, NULL, 0);
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
    for (i=0; i < 1000; i++) {
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
  shmdt(raw_data);
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
