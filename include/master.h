#ifndef MASTER_H
#define MASTER_H
#include "idealEngine.h"

void initMaster();
void delegateTasks();
void populateShm(int workerID, int taskID, int workerType);

#endif
