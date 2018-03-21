#include "idealEngine.h"

master_data masterInfo;

//partition_bounds bound[NUM_PARTITIONS_REDUCE]

void initMaster() {
  int i,j;
  char nameBuffer[20];
  long int **bound;
  //  char **filenames;
  key_t shmkey;
  int shmid;
  int idCount=1;
  printf("entered initMaster\n");
  bound = find_partition_bounds();
  //get user allocated partition bounds
  printf("found partition bounds\n");

  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    masterInfo.mapInfo[i].mapBound.start=bound[i][0];
    masterInfo.mapInfo[i].mapBound.end=bound[i][1];
    free(bound[i]); //Free partition bounds
  }
  //Free partition bounds
  free(bound);

  /* //sanity check
     for (i=0;i<NUM_PARTITIONS_MAP;i++) {
     printf("%d",masterInfo.mapInfo[i].mapBound.start);
     printf(",%d\n",masterInfo.mapInfo[i].mapBound.end);
     }*/
  //  filenames = specify_intermediate_filenames();
  for (i=0;i<NUM_WORKERS;i++) {
    shmkey=ftok(exe_name,idCount);
    shmid = shmget (shmkey, sizeof (int)*4, 0644 | IPC_CREAT);
    if (shmid < 0){ //shared memory error check
      perror ("shmget\n");
      exit (1);
    }
    printf("Shared memory allocated for worker in master (ID,shmid,shmkey:%d,%d,%d\n",idCount,shmid,shmkey);
    //attach to shared memory
    masterInfo.workerInfo[i].mmapping = (int *) shmat (shmid, NULL, 0);
    //populate the master data structure with useful worker process
    //information
    masterInfo.workerInfo[i].worker_type=&(masterInfo.workerInfo[i].mmapping[0]);
    masterInfo.workerInfo[i].status=&(masterInfo.workerInfo[i].mmapping[1]);
    masterInfo.workerInfo[i].task_index=&(masterInfo.workerInfo[i].mmapping[2]);
    *(masterInfo.workerInfo[i].worker_type)=63;
    *(masterInfo.workerInfo[i].task_index)=255;
    *(masterInfo.workerInfo[i].status)=WORKING;
    idCount++;
  }
  
  
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    
    shmkey=ftok(exe_name,idCount);
    shmid = shmget (shmkey, sizeof (long int)*2*(NUM_PARTITIONS_REDUCE+1), 0644 | IPC_CREAT);
    if (shmid < 0){ //shared memory error check
      perror ("shmget\n");
      exit (1);
    }
    printf("Shared memory allocated for mapper in master (ID,shmid,shmkey:%d,%d,%d\n",idCount,shmid,shmkey);
    masterInfo.mapInfo[i].shmBuffer = (long int *) shmat (shmid, NULL, 0);
    //give each mapper its start and end splice bounds from the
    //original input dataset
    masterInfo.mapInfo[i].shmBuffer[0]=masterInfo.mapInfo[i].mapBound.start;
    masterInfo.mapInfo[i].shmBuffer[1]=masterInfo.mapInfo[i].mapBound.start;
    idCount++;
  }


  for (i=0;i<NUM_PARTITIONS_REDUCE;i++) {
    
    shmkey=ftok(exe_name,idCount);
    shmid = shmget (shmkey, sizeof (long int)*2*(NUM_PARTITIONS_MAP), 0644 | IPC_CREAT);
    if (shmid < 0){ //shared memory error check
      perror ("shmget\n");
      exit (1);
    }
    printf("Shared memory allocated for reducer in master (ID,shmid,shmkey:%d,%d,%d\n",idCount,shmid,shmkey);
    
    masterInfo.reduceInfo[i].shmBuffer = (long int *) shmat (shmid, NULL, 0);
    //Can't populate, partitions must be provided by the mapper
    
    idCount++;
  }

  shmkey=ftok(exe_name,idCount);
  shmid = shmget (shmkey, sizeof (char)*255*NUM_PARTITIONS_MAP, 0644 | IPC_CREAT);
  if (shmid < 0){ //shared memory error check
    perror ("shmget\n");
    exit (1);
  }
  for (i=0;i<NUM_WORKERS;i++) {
    
    printf("Shared memory allocated filenames in master (ID,shmid,shmkey:%d,%d,%d\n",idCount,shmid,shmkey);
    
    masterInfo.workerInfo[i].mmapping_filenames = (char *) shmat (shmid, NULL, 0);
  }
  idCount++;
  
  
  /*

    masterInfo = malloc(sizeof(master_data));
    masterInfo->mapInfo=malloc(sizeof(mapper_data *)*NUM_PARTITIONS_MAP);
    masterInfo->reduceInfo=malloc(sizeof(reducer_data *)*NUM_PARTITIONS_REDUCE);
    masterInfo->workerInfo=malloc(sizeof(worker_data *)*NUM_WORKERS);
    printf("did master\n");
  
    for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    printf("inside 1\n");
    printf("%03d\n",masterInfo->);
    masterInfo->mapInfo[i]->mapBound=malloc(sizeof(partition_bounds));
    masterInfo->mapInfo[i]->reduceBounds=malloc(sizeof(partition_bounds *) * NUM_PARTITIONS_MAP);
    for (j=0;j<NUM_PARTITIONS_REDUCE;j++) {
    printf("inside 1\n");
    masterInfo->mapInfo[i]->reduceBounds[j]=malloc(sizeof(partition_bounds));
    }
    }
    printf("did map\n");
  
    for (i=0;i<NUM_PARTITIONS_REDUCE;i++) {
    for (j=0;j<NUM_PARTITIONS_MAP;j++) {
    masterInfo->reduceInfo[i]->reduceBounds[j]=malloc(sizeof(partition_bounds));
    }
    }
    printf("did reduce\n");
  
    for (i=0;i<NUM_WORKERS;i++) {
    masterInfo->workerInfo[i]->worker_type=malloc(sizeof(int *));
    masterInfo->workerInfo[i]->status=malloc(sizeof(int *));
    masterInfo->workerInfo[i]->task_index=malloc(sizeof(int *));
    masterInfo->workerInfo[i]->worker_index=malloc(sizeof(int *));
    }  
    printf("did workers\n");
  
    printf("hi\n");
    for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    printf("hi inside 1\n");
    //*(masterInfo->mapInfo[i]->mapBound->start)=0;//bound[i].start;
    printf("hi inside 2\n");
    //*(masterInfo->mapInfo[i]->mapBound->end)=*(bound[i].end);
    }
    printf("hi\n");
  */
}

void delegateTasks() {
  int i;
  sem_t *workers_not_empty;
  sem_t *workers_not_full;
  sem_t *workers_mutex;
  sem_t *worker_sem_tmp;
  char *workers_not_empty_str = "wksActive";
  char *workers_not_full_str = "wkscompleted";
  char *workers_mutex_str = "wksMutex";
  int freeWorker;
  int deployedMap;
  
  workers_not_empty = setup_sem(0,workers_not_empty_str);
  workers_not_full = setup_sem(0,workers_not_full_str);
  workers_mutex = setup_sem(1,workers_mutex_str);

  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    //    printf("status:%d\n",masterInfo->mapInfo[i]->status);
    //if(masterInfo->mapInfo[i]->status == IDLE){

    printf("assign a worker to mapping\n");
      
    //}
    
  }
  //Define heads
  simpleArray * workersAvailable = allocBuff(NUM_WORKERS);
  simpleArray * workersUnavailable = allocBuff(NUM_WORKERS);

  
  simpleArray * mappersAvailable = allocBuff(NUM_WORKERS);
  simpleArray * mappersUnavailable = allocBuff(NUM_WORKERS);
  
  simpleArray * reducersAvailable = allocBuff(NUM_WORKERS);
  simpleArray * reducersUnavailable = allocBuff(NUM_WORKERS);
  simpleArray * reducersComplete = allocBuff(NUM_WORKERS);
  
  //Magic Happens here
  //
  //
  
  //Map Phase
  printf("Begin map phase.\n");
  //While there are still remaining mappings
  while (!isEmpty(mappersAvailable)) {
    
    //While there are no avaliable workers, wait for a signal
    //that a worker has completed
    if (isEmpty(workersAvailable)) {
      //find a worker that finished and move it to the
      //Available queue, also move the corresponding map
      //to the Complete queue 
      sem_wait(workers_not_empty);
      //assuming the worker who won grabbed the lock
      freeWorker=*(masterInfo.finished);
      
      //worker now available
      push(workersAvailable,freeWorker);

      //worker no longer unavailable
      removeByValue(workersUnavailable,freeWorker);
      sem_post(workers_mutex);
    }
    //There is at least 1 worker that is free to do a map

    //Choose a worker to give a job
    freeWorker=pop(workersAvailable);
    deployedMap=pop(mappersAvailable);
    //Populate the shared memory of that worker with the
    //up-to-date copy from master
    
    //Move the mapper from the Available to Unavailable queue
    push(mappersUnavailable,deployedMap);
    
    //Signal that worker to wake up
    worker_sem_tmp=open_existing_sem(masterInfo.workerInfo[freeWorker].sem_start_str);
    sem_post(worker_sem_tmp);
  }

  //All mappings have been scheduled, wait for leftovers to finish
  while (!isEmpty(mappersUnavailable)) {
    //implement this
    break;
  }



  deallocBuff(workersAvailable);
  deallocBuff(workersUnavailable);
  
  deallocBuff(mappersAvailable);
  deallocBuff(mappersUnavailable);
  
  deallocBuff(reducersAvailable);
  deallocBuff(reducersUnavailable);
  deallocBuff(reducersComplete);
  
  cleanup_sem(workers_not_empty,workers_not_empty_str);
  cleanup_sem(workers_not_full,workers_not_full_str);
  cleanup_sem(workers_mutex,workers_mutex_str);
}
/*
  int findFinishedWorker () {
  //&WorkersAvailableQueue, entries
  return 0;
  }*/

void mapTask (partition_bounds* bound) {
  printf("Mapping\n");
}
void reduceTask (partition_bounds* bound) {
  printf("Reducing\n");
}

