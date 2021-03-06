#include "worker.h"
#include <math.h>
#include "shmWrapper.h"
#include "userFuncts.h"
#include "idealEngineTypes.h"
worker_data workerInfo;
char * input_filename = "input.dat";

void work(int workerID) {
  printf("Entering worker loop\n");
  sleep(1);
  key_t shmkey;
  int shmid;
  unsigned char *raw_data;
  unsigned char *raw_finished;
  char *workers_not_empty_str = "wksActive";
  char *sem_start_reduce_str = "wksReduce";
  sem_t *workers_not_empty;
  sem_t *sem_start_reduce;
  sem_t *sem_start;
  char *worker_tmp_name;
  int i,sum;
  long int * map_bounds;
  kv_pairs * key_val_pairs;

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
  sem_start_reduce=open_existing_sem(sem_start_reduce_str);
  
  worker_tmp_name=malloc(sizeof(char)*255);
  sprintf(worker_tmp_name,"worker%03d",workerID-1);
  sem_start = open_existing_sem(worker_tmp_name);
  free(worker_tmp_name);

  srand(0);
  while (1) {
    sem_wait(sem_start);
    printf("Mapper %d woke up to do %d\n",workerID,*(workerInfo.task_index));
    //printf("workerInfo.finished[NUM_WORKERS] = %d\n",workerInfo.finished[NUM_WORKERS]);
    if (workerInfo.finished[NUM_WORKERS]==1) {
      //printf("Mapper %d broke out\n",workerID);
      break;
    }


    map_bounds=(long int *)shmat(attachShm(NUM_WORKERS+NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+3),NULL,0);

    //critical section

    
    partition_bounds bounds;
    bounds.start=map_bounds[*(workerInfo.task_index)*2];
    bounds.end  =map_bounds[*(workerInfo.task_index)*2+1];
    
    //provide mapper key/value inputs
    key_val_pairs = produce_map_kvs(*(workerInfo.task_index),&bounds);
    printf("made it here %d\n",key_val_pairs->count);
    //
    map(key_val_pairs);
    
    printf("made it here too\n");
    //free mapper key/value pairs
    for (i = 0; i < key_val_pairs->count; i++) {
      free(key_val_pairs->pairs[i]);
    }
    free(key_val_pairs->pairs);
    free(key_val_pairs);
      
    workerInfo.finished[workerID-1]=1;
    
    //printf(":%s posted %d\n",workers_not_empty_str,workerID);
    printProgress(workerID);
    sem_post(workers_not_empty);
    //signal finished
  }

  //printf("Stuck before sem_start_reduce %d\n",workerID);
  sem_wait(sem_start_reduce);
  while (1) {
    //wait on start signal
    //printf(":sem_start wait %d\n",workerID);
    sem_wait(sem_start);
    //printf("--Reducer %d woke up\n",workerID);
    if (workerInfo.finished[NUM_WORKERS]==1) {
      //printf("Reducer %d broke out\n",workerID);
      break;
    }

    //critical section
    reduce();
    
    workerInfo.finished[workerID-1]=1;
    
    printProgress(workerID);
    sem_post(workers_not_empty);
    //signal finished
  }

  

  printf("Reduce phase complete: worker %d\n",workerID);
  //detach from worker shared memory
  detachShm(workerID,raw_data);
  detachShm(NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+NUM_WORKERS+2,raw_finished);
  printf("Worker %d detached from Shm\n",workerID);
}


void printProgress(int ID) {
  char str[NUM_WORKERS+1];
  int i;
  for (i = 0; i < NUM_WORKERS; i++) {
    str[i]=workerInfo.finished[i] ? '1'+i : ' ';
  }
  str[NUM_WORKERS]='\0';
  printf("worker %d status: [%s] %d\n",ID,str,workerInfo.finished[NUM_WORKERS]);
}

int fib(int n){
  return n<=2 ? 1 : fib(n-1)+fib(n-2);
}
