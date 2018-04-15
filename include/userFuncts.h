#ifndef USERFUNCTS_H
#define USERFUNCTS_H
#include "idealEngine.h"

void map ();
void reduce ();
long int ** find_partition_bounds(char *inputFilename);
void inputReader(char *);
char ** specify_intermediate_filenames();

#endif
