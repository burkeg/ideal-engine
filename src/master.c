#include "idealEngine.h"

mapper_data mapInfo[NUM_PARTITIONS_MAP];
reducer_data reduceInfo[NUM_PARTITIONS_REDUCE];

void initMapper(partition_bounds* bound) {
  int i;
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    mapInfo[i].mapBound.start=bound[i].start;
    mapInfo[i].mapBound.end=bound[i].end;
    mapInfo[i].in_use=0;
    mapInfo[i].status=IDLE;
    
  }
  
}

void delegateTasks() {
  int i;
  //mapTask(NULL);
  //reduceTask(NULL);

  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    if((mapInfo[i].in_use == 0) && (mapInfo[i].status == IDLE)){

      printf("assign a worker to mapping\n");
      
    }
    
  }
}

void mapTask (partition_bounds* bound) {
  printf("Mapping\n");
}
void reduceTask (partition_bounds* bound) {
  printf("Reducing\n");
}

