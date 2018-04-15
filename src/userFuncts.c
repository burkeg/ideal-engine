#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "idealEngine.h"

void map () {
  int sum,i,lim;
  sum=0;
  //  lim=10000000+rand()%10000000;
  lim=500;
  for (i=0; i < lim; i++) {
    sum += sin((double)i);
    //i+=rand()%5;
  }
}

void reduce () {
  int sum,i,lim;
  sum=0;
  //  lim=10000000+rand()%10000000;
  lim=500;
  for (i=0; i < lim; i++) {
    sum += sin((double)i);
    //i+=rand()%5;
  }
}

/*
  Must return NUM_PARTITIONS_MAP long int *
  where each long int * has 2 elements, 
  start (inclusive), end (exclusive)
  
  start and end are the byte locations that
  partition the input file into each
  NUM_PARTITIONS_MAP segments.

  http://www.cplusplus.com/reference/cstdio/ftell/
*/
long int ** find_partition_bounds() {
  long int ** bounds;
  int i;
  bounds = malloc(sizeof(long int) * NUM_PARTITIONS_MAP);
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    bounds[i]=malloc(sizeof(long int) * 2);
    bounds[i][0]=i*2;
    bounds[i][1]=i*2+1;
  }
  return bounds;
}

void inputReader(char *inputFilename) {
  FILE * fp;
  if ((fp=fopen(inputFilename,"r"))==NULL) {
    printf("failed to read input file\n");
    return;
  }
  
  
}
/*
  Must return NUM_PARTITIONS_MAP strings
  each string can be up to 255 characters long
*/
char ** specify_intermediate_filenames() {
  char ** names;
  int i;
  names = malloc(sizeof(char *) * NUM_PARTITIONS_MAP);
  for (i=0;i<NUM_PARTITIONS_MAP;i++) {
    names[i]=malloc(sizeof(char)*255);
    sprintf(names[i],"intermediate_%03d",i);
  }
  return names;
}
