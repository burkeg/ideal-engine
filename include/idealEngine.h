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
#include <sys/queue.h>
#include "master.h"
#include "worker.h"
#include "semUtils.h"
#include "userFuncts.h"
#include "simpleArray.h"
#include "idealEngineTypes.h"
#include "shmWrapper.h"

void work();
char * input_filename;
char * exe_name;

#endif
