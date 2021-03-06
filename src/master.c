#include "idealEngine.h"
#include "shmWrapper.h"

master_data masterInfo;


void initMaster() {
  int i,j;
  char nameBuffer[20];
  long int **bound;
  //  char **filenames;
  int idCount=1;
  bound = inputReader("input.dat");
  printf("entering initMaster\n");
  //get user allocated partition bounds


  /* //sanity check
     for (i=0;i<NUM_PARTITIONS_MAP;i++) {
     printf("%d",masterInfo.mapInfo[i].mapBound.start);
     printf(",%d\n",masterInfo.mapInfo[i].mapBound.end);
     }*/
  int tmp;
  tmp=idCount;
  //Worker data
  createShm(sizeof(long int)*2*NUM_PARTITIONS_MAP,NUM_WORKERS+NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+3);
  for (i=0;i<NUM_WORKERS;i++) {
    createShm(sizeof (int)*4,idCount);

    masterInfo.workerInfo[i].mmapping = (int *) shmat (attachShm(idCount), NULL, 0);
    masterInfo.workerInfo[i].mapper_bounds = (long int *) shmat (attachShm(NUM_WORKERS+NUM_PARTITIONS_MAP+NUM_PARTITIONS_REDUCE+3), NULL, 0);

    //populate the master data structure with useful worker process
    //information
    masterInfo.workerInfo[i].worker_type=&(masterInfo.workerInfo[i].mmapping[0]);
    masterInfo.workerInfo[i].task_index=&(masterInfo.workerInfo[i].mmapping[2]);
    *(masterInfo.workerInfo[i].worker_type)=63;
    *(masterInfo.workerInfo[i].task_index)=255;
    idCount++;
  }
  
  printf("Worker Data: [%d,%d)\n",tmp, idCount);
  


  tmp=idCount;


  //Mapper partition data
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    createShm(sizeof (long int)*2*(NUM_PARTITIONS_REDUCE+1),idCount);
    
    masterInfo.mapInfo[i].shmBuffer = (long int *) shmat (attachShm(idCount), NULL, 0);
    //give each mapper its start and end splice bounds from the
    //original input dataset
    masterInfo.mapInfo[i].shmBuffer[0]=bound[i][0];
    masterInfo.mapInfo[i].shmBuffer[1]=bound[i][1];
    //printf("Placing start,end () into ");
    idCount++;
  }

  printf("Mapper Data: [%d,%d)\n",tmp, idCount);


  tmp=idCount;
  
  //Reducer Partition data
  for (i=0;i<NUM_PARTITIONS_REDUCE;i++) {
    createShm(sizeof (long int)*2*(NUM_PARTITIONS_MAP),idCount);
    
    masterInfo.reduceInfo[i].shmBuffer = (long int *) shmat (attachShm(idCount), NULL, 0);
    //Can't populate, partitions must be provided by the mapper
    
    idCount++;
  }

  printf("Reducer Data: [%d,%d)\n",tmp, idCount);


  tmp=idCount;
  //Intermediate Filenames
  createShm(sizeof (char)*255*NUM_PARTITIONS_MAP, idCount);
  
  for (i=0;i<NUM_WORKERS;i++) {
    
    
    masterInfo.workerInfo[i].mmapping_filenames = (char *) shmat (attachShm(idCount), NULL, 0);
  }
  idCount++;

  printf("Intermediate Filename Data: [%d,%d)\n",tmp, idCount);

  tmp=idCount;
  //Finished Flag
  createShm(sizeof (int)*(NUM_WORKERS+1), idCount);
  for (i=0;i<NUM_WORKERS;i++) {
    
    masterInfo.workerInfo[i].finished = (int *) shmat (attachShm(idCount), NULL, 0);
    masterInfo.workerInfo[0].finished[i]=0;
  }
  //the final finished flag indicates when the current phase is complete.
  masterInfo.workerInfo[0].finished[i]=0;
  
  masterInfo.finished = (int *) shmat (attachShm(idCount), NULL, 0);
  idCount++;

  printf("Finished flag: [%d,%d)\n",tmp, idCount);



  //masterInfo.workerInfo[i].mapper_bounds;

  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    masterInfo.workerInfo[0].mapper_bounds[i*2]=bound[i][0];
    masterInfo.workerInfo[0].mapper_bounds[i*2+1]=bound[i][1];
    free(bound[i]); //Free partition bounds
  }
  //Free partition bounds
  free(bound);
  
}

void delegateTasks() {
  int i;
  int idCount;
  int tmp;
  sem_t *workers_not_empty;
  sem_t *sem_start_reduce;
  sem_t *sem_worker_can_start[NUM_WORKERS];
  char *workers_not_empty_str = "wksActive";
  char *sem_start_reduce_str = "wksReduce";
  char *worker_tmp_name;
  int freeWorker;
  int deployedMap;
  int deployedReduce;
  int sem_value;
  
  workers_not_empty = setup_sem(0,workers_not_empty_str);
  sem_start_reduce = setup_sem(0,sem_start_reduce_str);
  
  for (i=0;i<NUM_WORKERS;i++) {
    worker_tmp_name=malloc(sizeof(char)*255);
    sprintf(worker_tmp_name,"worker%03d",i);
    sem_worker_can_start[i] = setup_sem(0,worker_tmp_name);
    free(worker_tmp_name);
  }

  //Define heads
  simpleArray * workersAvailable = allocBuff(NUM_WORKERS);
  simpleArray * workersUnavailable = allocBuff(NUM_WORKERS);

  
  simpleArray * mappersAvailable = allocBuff(NUM_PARTITIONS_MAP);
  simpleArray * mappersUnavailable = allocBuff(NUM_PARTITIONS_MAP);
  
  simpleArray * reducersAvailable = allocBuff(NUM_PARTITIONS_REDUCE);
  simpleArray * reducersUnavailable = allocBuff(NUM_PARTITIONS_REDUCE);
  simpleArray * reducersComplete = allocBuff(NUM_PARTITIONS_REDUCE);

  //Populate buffers
  for (i=0;i<NUM_WORKERS;i++) {
    push(workersAvailable,i);
  }
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    push(mappersAvailable,i);
  }
  for (i=0;i<NUM_PARTITIONS_REDUCE;i++) {
    push(reducersAvailable,i);
  }
  //Magic Happens here
  //
  //
  sleep(1);//wait for workers to get ready
  //Map Phase
  printf("Begin map phase.\n");
  //While there are still remaining mappings
  while (!isFull(mappersUnavailable)) {
    //printf("Master (avail %d) (unavail %d)\n",len(mappersAvailable),len(mappersUnavailable));
    //While there are no avaliable workers, wait for a signal
    //that a worker has completed
    printf("### mappers Available\n");
    if (isEmpty(workersAvailable)) {
      //printf("### No workers available\n");
      //find a worker that finished and move it to the
      //Available queue, also move the corresponding map
      //to the Complete queue
      //printf(":%s wait entered\n",workers_not_empty_str);
      sem_wait(workers_not_empty);
      //printf(":%s wait exited\n",workers_not_empty_str);
      
      for (i=0;i<NUM_WORKERS;i++) {
	//	printf("masterLoop finished[%d]=%d\n",i,masterInfo.finished[i]);
	if (masterInfo.finished[i] != 0) {
	  break;
	}
      }
      if (i==NUM_WORKERS) {
	printf("If this prints, I'm a bad programmer.\n");
      }
      masterInfo.workerInfo[0].finished[i]=0;
      //mapper now unavailable
      //printf("----Mapper %d done.\n",*(masterInfo.workerInfo[i].task_index));
      //push(mappersUnavailable,*(masterInfo.workerInfo[i].task_index));
      //printf("mappersUnavailable ");
      //printBuff(mappersUnavailable);
      //worker no longer available
      removeByValue(workersAvailable,*(masterInfo.workerInfo[i].task_index));
      //      printf("workersAvailable ");
      //      printBuff(workersAvailable);
      
      //worker now available
      push(workersAvailable,i);
      //      printf("workersAvailable ");
      //      printBuff(workersAvailable);

      //worker no longer unavailable
      removeByValue(workersUnavailable,i);
      //      printf("workersUnavailable ");
      //      printBuff(workersUnavailable);
      
      //printf("### Worker %d now free.\n",i);
    }
    //There is at least 1 worker that is free to do a map

    //Choose a worker to give a job
    freeWorker=pop(workersAvailable);
    //    printf("workersAvailable ");
    //    printBuff(workersAvailable);
    deployedMap=pop(mappersAvailable);
    //    printf("mappersAvailable ");
    //    printBuff(mappersAvailable);
    
    //printf("### Worker %d selected to do mapping %d.\n",freeWorker,deployedMap);
    //Populate the shared memory of that worker with the
    //up-to-date copy from master
    populateShm(freeWorker,deployedMap,MAPPER);
    
    //Move the mapper from the Available to Unavailable queue
    push(mappersUnavailable,deployedMap);
    //    printf("mappersUnavailable ");
    //    printBuff(mappersUnavailable);
    //printf("------Scheduling mapper %d\n",deployedMap);
    //Signal that worker to wake up
    sem_post(sem_worker_can_start[freeWorker]);
  }
  int flag;
  
  while(1) {
    flag=0;
    for (i=0;i<NUM_WORKERS;i++){
      if (masterInfo.workerInfo[0].finished[i] == 0) {
	flag=1;
      }
    }
    if (flag==0)
      break;
  }
  printf("ALL MAPPERS SCHEDULED\n");
  //All mappings have been scheduled, wait for leftovers to finish
  masterInfo.workerInfo[0].finished[NUM_WORKERS]=1;
  for (i=0;i<NUM_WORKERS;i++) {
    sem_post(sem_worker_can_start[i]);    
  }
  printf("ALL MAPPERS DONE\n");
  //REDUCE
  printf("start 2 second wait,\n");
  sleep(2);
  printf("end 2 second wait,\n");
  //Repopulate buffer
  for (i=0;i<NUM_WORKERS;i++) {
    push(workersAvailable,i);
    masterInfo.workerInfo[i].finished[i] = 0;
  }
  masterInfo.workerInfo[0].finished[NUM_WORKERS] = 0;
  for (i=0;i<NUM_WORKERS;i++) {
    printf("signalling sem_start_reduce\n");
    sem_post(sem_start_reduce);    
  }

  sem_getvalue(workers_not_empty,&sem_value);
  printf("sem_value: %d\n",sem_value);
  for (i=0; i<NUM_WORKERS;i++) {
    sem_wait(workers_not_empty);
  }
  sem_getvalue(workers_not_empty,&sem_value);
  printf("sem_value: %d\n",sem_value);
  printBuff(workersAvailable);
  printBuff(workersUnavailable);
  printBuff(reducersAvailable);
  printBuff(reducersUnavailable);
  
  while (!isFull(reducersUnavailable)) {
    //While there are no avaliable workers, wait for a signal
    //that a worker has completed
    //printf("### mappers Available\n");
    if (isEmpty(workersAvailable)) {
      //printf("### No workers available\n");
      //find a worker that finished and move it to the
      //Available queue, also move the corresponding map
      //to the Complete queue
      sem_wait(workers_not_empty);
      //printf("master woke up\n");
      for (i=0;i<NUM_WORKERS;i++) {
	if (masterInfo.finished[i] != 0) {
	  break;
	}
      }
      if (i==NUM_WORKERS) {
	printf("If this prints, I'm a bad programmer.\n");
      }
      masterInfo.workerInfo[0].finished[i]=0;
      //mapper now unavailable
      
      //printf("reducersUnavailable ");
      //printBuff(reducersUnavailable);
      //worker no longer available
      removeByValue(workersAvailable,*(masterInfo.workerInfo[i].task_index));
      //printf("workersAvailable ");
      //printBuff(workersAvailable);
      
      //worker now available
      push(workersAvailable,i);
      //printf("workersAvailable ");
      //printBuff(workersAvailable);

      //worker no longer unavailable
      removeByValue(workersUnavailable,i);
      //printf("workersUnavailable ");
      //printBuff(workersUnavailable);
    }
    //There is at least 1 worker that is free to do a map

    //Choose a worker to give a job
    freeWorker=pop(workersAvailable);
    //printf("workersAvailable ");
    //printBuff(workersAvailable);
    deployedReduce=pop(reducersAvailable);
    //printf("reducersAvailable ");
    //printBuff(reducersAvailable);
    
    //printf("### Worker %d selected to do reducing %d.\n",freeWorker,deployedReduce);
    //Populate the shared memory of that worker with the
    //up-to-date copy from master
    populateShm(freeWorker,deployedReduce,REDUCER);
    
    //Move the mapper from the Available to Unavailable queue
    push(reducersUnavailable,deployedReduce);
    //Signal that worker to wake up
    sem_post(sem_worker_can_start[freeWorker]);
  }

  
 while(1) {
    flag=0;
    for (i=0;i<NUM_WORKERS;i++){
      if (masterInfo.workerInfo[0].finished[i] == 0) {
	flag=1;
      }
    }
    if (flag==0)
      break;
  }
  printf("ALL REDUCERS SCHEDULED\n");
  //All mappings have been scheduled, wait for leftovers to finish
  masterInfo.workerInfo[0].finished[NUM_WORKERS]=1;
  for (i=0;i<NUM_WORKERS;i++) {
    sem_post(sem_worker_can_start[i]);    
  }
  printf("ALL REDUCERS DONE\n");

  sleep(2);
  
  deallocBuff(workersAvailable);
  deallocBuff(workersUnavailable);
  
  deallocBuff(mappersAvailable);
  deallocBuff(mappersUnavailable);
  
  deallocBuff(reducersAvailable);
  deallocBuff(reducersUnavailable);
  deallocBuff(reducersComplete);
  
  //  shmctl (shmid, IPC_RMID, 0);
  
  cleanup_sem(workers_not_empty,workers_not_empty_str);
  
  
  for (i=0;i<NUM_WORKERS;i++) {
    worker_tmp_name=malloc(sizeof(char)*255);
    sprintf(worker_tmp_name,"worker%03d",i);
    cleanup_sem(sem_worker_can_start[i],worker_tmp_name);
    free(worker_tmp_name);
  }

  //Detach from all shm
  idCount=1;
  tmp=idCount;
  
  for (i=0;i<NUM_WORKERS;i++) {
    detachShm(idCount,masterInfo.workerInfo[i].mmapping);
    idCount++;
  }
  
  printf("Worker Data Detached: [%d,%d)\n",tmp, idCount);

  tmp=idCount;

  //Mapper partition data
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    detachShm(idCount,masterInfo.mapInfo[i].shmBuffer);
    idCount++;
  }

  printf("Mapper Data Detached: [%d,%d)\n",tmp, idCount);


  tmp=idCount;
  
  //Reducer Partition data
  for (i=0;i<NUM_PARTITIONS_REDUCE;i++) {
    detachShm(idCount,masterInfo.reduceInfo[i].shmBuffer);
    idCount++;
  }

  printf("Reducer Data Detached: [%d,%d)\n",tmp, idCount);


  tmp=idCount;
  //Intermediate Filenames
  
  for (i=0;i<NUM_WORKERS;i++) {
    detachShm(idCount,masterInfo.workerInfo[i].mmapping_filenames);
  }
  idCount++;

  printf("Intermediate Filename Data Detached: [%d,%d)\n",tmp, idCount);

  tmp=idCount;
  //Finished Flag
 
  for (i=0;i<NUM_WORKERS;i++) {
    detachShm(idCount,masterInfo.workerInfo[i].finished);
  }
  detachShm(idCount,masterInfo.finished);
  idCount++;

  printf("Finished flag Detached: [%d,%d)\n",tmp, idCount);

  for (i=1; i<=MAX_SHM_SEGMENTS; i++) {
    deleteShm(i);
  }
  printf("Finished removing all shared memory segments\n");
}

void populateShm(int workerID, int taskID, int workerType) {
  if (workerType == MAPPER) {
    *(masterInfo.workerInfo[workerID].worker_type)=workerType;
    *(masterInfo.workerInfo[workerID].task_index)=taskID;
  }
  else if (workerType == REDUCER) {
    *(masterInfo.workerInfo[workerID].worker_type)=workerType;
    *(masterInfo.workerInfo[workerID].task_index)=taskID;
    
  }
}
  
  
