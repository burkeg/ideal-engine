#include "idealEngine.h"

master_data masterInfo;


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

  
  //Worker data
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
    masterInfo.workerInfo[i].task_index=&(masterInfo.workerInfo[i].mmapping[2]);
    *(masterInfo.workerInfo[i].worker_type)=63;
    *(masterInfo.workerInfo[i].task_index)=255;
    idCount++;
  }
  




  //Mapper partition data
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



  
  //Reducer Partition data
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



  //Intermediate Filenames
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




  //Finished Flag
  shmkey=ftok(exe_name,idCount);
  shmid = shmget (shmkey, sizeof (int)*(NUM_WORKERS+1), 0644 | IPC_CREAT);
  if (shmid < 0){ //shared memory error check
    perror ("shmget\n");
    exit (1);
  }
  for (i=0;i<NUM_WORKERS;i++) {
    
    printf("Shared memory allocated for finished flag in workers (ID,shmid,shmkey:%d,%d,%d\n",idCount,shmid,shmkey);
    
    masterInfo.workerInfo[i].finished = (int *) shmat (shmid, NULL, 0);
    masterInfo.workerInfo[0].finished[i]=0;
  }
  //the final finished flag indicates when the current phase is complete.
  masterInfo.workerInfo[0].finished[i]=0;
  printf("Shared memory allocated for finished flag in master (ID,shmid,shmkey:%d,%d,%d\n",idCount,shmid,shmkey);
  
  masterInfo.finished = (int *) shmat (shmid, NULL, 0);
  idCount++;






  
}

void delegateTasks() {
  int i;
  sem_t *workers_not_empty;
  sem_t *sem_worker_can_start[NUM_WORKERS];
  char *workers_not_empty_str = "wksActive";
  char *worker_tmp_name;
  int freeWorker;
  int deployedMap;
  
  workers_not_empty = setup_sem(0,workers_not_empty_str);
  
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
  sleep(5);//wait for workers to get ready
  //Map Phase
  printf("Begin map phase.\n");
  //While there are still remaining mappings
  while (!isFull(mappersUnavailable)) {
    //printf("Master (avail %d) (unavail %d)\n",len(mappersAvailable),len(mappersUnavailable));
    //While there are no avaliable workers, wait for a signal
    //that a worker has completed
    //printf("### mappers Available\n");
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
  
  //  shmctl (shmid, IPC_RMID, 0);
  
  cleanup_sem(workers_not_empty,workers_not_empty_str);
  
  
  for (i=0;i<NUM_WORKERS;i++) {
    worker_tmp_name=malloc(sizeof(char)*255);
    sprintf(worker_tmp_name,"worker%03d",i);
    cleanup_sem(sem_worker_can_start[i],worker_tmp_name);
    free(worker_tmp_name);
  }
}

void populateShm(int workerID, int taskID, int workerType) {
  if (workerType == MAPPER) {
    *(masterInfo.workerInfo[workerID].worker_type)=workerType;
    *(masterInfo.workerInfo[workerID].task_index)=taskID;
    *(masterInfo.workerInfo[workerID].worker_type)=workerType;
  }
  else if (workerType == REDUCER) {
    
  }
}
  
  
