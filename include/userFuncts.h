#ifndef USERFUNCTS_H
#define USERFUNCTS_H
#include "idealEngine.h"
#include "idealEngineTypes.h"
void map (kv_pairs *);
void reduce ();
long int ** find_partition_bounds();
long int ** inputReader(char *);
int findFirstNewline(char * stringData, int len);
char ** specify_intermediate_filenames();
kv_pairs * produce_map_kvs(int mapID,partition_bounds * bounds);


#endif
