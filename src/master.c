#include "idealEngine.h"

master_data masterInfo;

//partition_bounds bound[NUM_PARTITIONS_REDUCE]

void initMaster() {
  int i,j;
  char nameBuffer[20];
  long int **bound;
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

  for (i=0;i<NUM_WORKERS;i++) {
    shmkey=ftok(exe_name,idCount);
    shmid = shmget (shmkey, sizeof (int)*4, 0644 | IPC_CREAT);
    if (shmid < 0){ //shared memory error check
      perror ("shmget\n");
      exit (1);
    }
    //attach to shared memory
    masterInfo.workerInfo[i].mmapping = (int *) shmat (shmid, NULL, 0);
    masterInfo.workerInfo[i].worker_type=&(masterInfo.workerInfo[i].mmapping[0]);
    masterInfo.workerInfo[i].status=&(masterInfo.workerInfo[i].mmapping[1]);
    masterInfo.workerInfo[i].task_index=&(masterInfo.workerInfo[i].mmapping[2]);
    masterInfo.workerInfo[i].worker_index=&(masterInfo.workerInfo[i].mmapping[3]);
    idCount++;
  }
  
  
  
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
  //mapTask(NULL);
  //reduceTask(NULL);

  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    //    printf("status:%d\n",masterInfo->mapInfo[i]->status);
    //if(masterInfo->mapInfo[i]->status == IDLE){

    printf("assign a worker to mapping\n");
      
    //}
    
  }
}

void mapTask (partition_bounds* bound) {
  printf("Mapping\n");
}
void reduceTask (partition_bounds* bound) {
  printf("Reducing\n");
}

