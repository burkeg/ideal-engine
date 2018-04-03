#ifndef SIMPLEARRAY_H
#define SIMPLEARRAY_H
#include "idealEngine.h"

typedef struct simpleArray {
  int size;
  int *data;
  int head;
} simpleArray;

simpleArray * allocBuff(int capacity);
int len(simpleArray * buffer);
int peek(simpleArray * buffer);
int pop(simpleArray * buffer);
void push(simpleArray * buffer,int value);
int isEmpty(simpleArray * buffer);
int isFull(simpleArray * buffer);
void deallocBuff(simpleArray * buffer);
void removeByValue(simpleArray * buffer, int value);
void printBuff(simpleArray * buffer);
void demo();

#endif
