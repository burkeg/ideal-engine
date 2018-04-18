#ifndef IDEAL_ENGINE_TYPES_H
#define IDEAL_ENGINE_TYPES_H


#define NUM_PARTITIONS_MAP 8
#define NUM_PARTITIONS_REDUCE 16
#define NUM_WORKERS 3

//Worker status
#define IDLE 0
#define WORKING 1
#define COMPLETE 2

//Worker type
#define NONE 0
#define MAPPER 1
#define REDUCER 2

#define WORKER_SHM_SIZE 3
#define MAPPER_SHM_SIZE 4
#define REDUCER_SHM_SIZE 4
#define FILENAMES_SHM_SIZE 4

#define MAX_VALUE_LENGTH 20
#define MAX_KEYS_PER_MAPPER 1000

typedef struct kv_pair {
  long int key;
  char value[MAX_VALUE_LENGTH];
} kv_pair;

typedef struct kv_pairs {
  int count;
  kv_pair ** pairs;
} kv_pairs;


typedef struct partition_bounds {
  long int start;
  long int end;
} partition_bounds;

typedef struct mapper_data {
  partition_bounds mapBound;
  partition_bounds reduceBounds[NUM_PARTITIONS_REDUCE];
  long int * shmBuffer;
} mapper_data;


typedef struct reducer_data {
  partition_bounds reduceBounds[NUM_PARTITIONS_MAP];
  long int * shmBuffer; 
} reducer_data;

typedef struct worker_data {
  int *worker_type;                   //points to mmapping
  int *task_index;                    //points to mmapping
  char *workers_not_empty;            //unique to workers
  int *finished;                      //shared
  char *mmapping_filenames;           //shared
  int *mmapping;                      //shared
  long int *mapper_bounds;            //shared
} worker_data;

typedef struct master_data {
  mapper_data mapInfo[NUM_PARTITIONS_MAP];
  reducer_data reduceInfo[NUM_PARTITIONS_REDUCE];
  worker_data workerInfo[NUM_WORKERS];
  int *finished;
} master_data;

#endif
