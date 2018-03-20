#ifndef SEMUTILS_H
#define SEMUTILS_H
#include "idealEngine.h"

sem_t* setup_sem(int value, char * name);
void cleanup_sem(sem_t* sem, char* name);
void barrier(sem_t* sem,sem_t* mutex, int* counter, int n);
#endif
