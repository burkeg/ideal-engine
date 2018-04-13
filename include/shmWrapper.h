#ifndef SHMWRAPPER_H
#define SHMWRAPPER_H

#define MAX_SHM_SEGMENTS 10

typedef struct shmData {
  key_t shmkey;
  int shmid;
  int numUsing;
} shmData;

void setupShmWrapper();
void initShmWrapper(char * exe_name);
void createShm(size_t size,int ID);
void *attachShm(int ID);
void detachShm(int ID,void * ptr);
void deleteShm(int ID);
void closeShmWrapper();
#endif
