#include <record.h>
#include <limits.h>

#include <ht_table.h>
#include <sht_table.h>

#define MAX_BYTES 25     // Consider maximum length of a surname being 30
#define MAX_BUCKETS 20


int HashStatistics(char* filename);