#ifndef IDEAL_ENGINE_H
#define IDEAL_ENGINE_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include  <sys/types.h>
#include "master.h"
#include "worker.h"

typedef struct partition_bounds {
   long int start;
   long int end;
} partition_bounds;


#endif
