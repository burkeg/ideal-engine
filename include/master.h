#ifndef MASTER_H
#define MASTER_H
#include "idealEngine.h"

#define NUM_PARTITIONS_MAP 4
#define NUM_PARTITIONS_REDUCE 4

typedef struct partition_bounds {
   long int start;
   long int end;
} partition_bounds;

void delegateTasks();
void mapTask (partition_bounds* bound);
void reduceTask (partition_bounds* bound);
#endif
