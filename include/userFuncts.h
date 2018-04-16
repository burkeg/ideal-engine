#ifndef USERFUNCTS_H
#define USERFUNCTS_H
#include "idealEngine.h"

void map ();
void reduce ();
long int ** find_partition_bounds();
void inputReader(char *);
int findFirstNewline(char * stringData, int len);
char ** specify_intermediate_filenames();

#endif
