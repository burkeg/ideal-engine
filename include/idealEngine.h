#ifndef IDEAL_ENGINE_H
#define IDEAL_ENGINE_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/shm.h>
#include "master.h"
#include "worker.h"
#include "semUtils.h"

#define NUM_PARTITIONS_MAP 4
#define NUM_PARTITIONS_REDUCE 4

#define IDLE 0
#define WORKING 1
#define COMPLETE 2

typedef struct partition_bounds {
  long int start;
  long int end;
} partition_bounds;

typedef struct mapper_data {
  partition_bounds mapBound;
  partition_bounds reduceBounds[NUM_PARTITIONS_REDUCE];
  int status; //0=Idle,   1=working,   2=complete
  int in_use;
} mapper_data;

typedef struct reducer_data {
  partition_bounds bound;
  FILE* fp;
} reducer_data;


void initMapper(partition_bounds bound[NUM_PARTITIONS_REDUCE]);
void delegateTasks();
void mapTask (partition_bounds* bound);
void reduceTask (partition_bounds* bound);


sem_t* setup_sem(int value, char * name);
void cleanup_sem(sem_t* sem, char* name);

void work();
#endif
