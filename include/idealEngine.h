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
#include <sys/ipc.h>
#include "master.h"
#include "worker.h"
#include "semUtils.h"
#include "userFuncts.h"

#define NUM_PARTITIONS_MAP 4
#define NUM_PARTITIONS_REDUCE 4
#define NUM_WORKERS 8

//Worker status
#define IDLE 0
#define WORKING 1
#define COMPLETE 2

//Worker type
#define NONE 0
#define MAPPER 1
#define REDUCER 2

#define WORKER_SHM_SIZE 4
#define MAPPER_SHM_SIZE 4
#define REDUCER_SHM_SIZE 4
#define FILENAMES_SHM_SIZE 4

typedef struct partition_bounds {
  long int start;
  long int end;
} partition_bounds;

typedef struct mapper_data {
  partition_bounds mapBound;
  partition_bounds reduceBounds[NUM_PARTITIONS_REDUCE];
  char *mmapping;
} mapper_data;


typedef struct reducer_data {
  partition_bounds reduceBounds[NUM_PARTITIONS_MAP];
  char *mmapping;
} reducer_data;

typedef struct worker_data {
  int *worker_type;
  int *status;
  int *task_index;
  int *worker_index;
  char *mmapping_filenames;
  int *mmapping;
} worker_data;

typedef struct master_data {
  mapper_data mapInfo[NUM_PARTITIONS_MAP];
  reducer_data reduceInfo[NUM_PARTITIONS_REDUCE];
  worker_data workerInfo[NUM_WORKERS];
} master_data;

char* exe_name;

void initMaster();
void delegateTasks();
void mapTask (partition_bounds* bound);
void reduceTask (partition_bounds* bound);
/*
sem_t* open_existing_sem(char * name);
sem_t* setup_sem(int value, char * name);
void cleanup_sem(sem_t* sem, char* name);
*/
void work();
#endif
