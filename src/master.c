#include "master.h"

void delegateTasks() {
  mapTask(NULL);
  reduceTask(NULL);
}

void mapTask (partition_bounds* bound) {
  printf("Mapping\n");
}
void reduceTask (partition_bounds* bound) {
  printf("Reducing\n");
}

