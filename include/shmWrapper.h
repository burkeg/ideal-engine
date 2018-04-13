#ifndef SHMWRAPPER_H
#define SHMWRAPPER_H

#define MAX_SHM_SEGMENTS 12000

typedef struct shmData {
  key_t shmkey;
  int shmid;
  int inuse;
} shmData;

void setupShmWrapper();
void initShmWrapper(char * exe_name);
void createShm(size_t size,int ID);
int  attachShm(int ID);
void detachShm(int ID,void * ptr);
void deleteShm(int ID);
void closeShmWrapper();
void closeShmWrapper();
#endif
