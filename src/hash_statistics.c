#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_statistics.h"
#include "bf.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }
  
int HashStatistics(char* filename){
  
  // Open file 
  int desc;
  CALL_OR_DIE(BF_OpenFile(filename, &desc));

  BF_Block* metadata;
  BF_Block_Init(&metadata);

  // Get metadata of the file from the first block
  CALL_OR_DIE(BF_GetBlock(desc, 0, metadata)); 
  void* identifier = (void*)BF_Block_GetData(metadata);
  HT_info* hash = (HT_info*)identifier;  
  SHT_info* sec_hash = (SHT_info*)identifier;

  int buckets; 
  int hash_block[MAX_BUCKETS];
  if(strcmp(hash->identifier, "HashTable") == 0){

    printf("\nHashtable\n");
    buckets = hash->buckets;                  // Number of buckets in the file
    for(int i=0; i<MAX_BUCKETS; i++)
      hash_block[i] = hash->hash_block[i];
    
    printf("hash_block address: %p\n", hash_block);

  } else if (strcmp(sec_hash->identifier, "Secondary HashTable") == 0){
    
    printf("\nSecondary Hashtable\n");
    buckets = sec_hash->buckets;                  // Number of buckets in the file
    for(int i=0; i<MAX_BUCKETS; i++)
      hash_block[i] = sec_hash->hash_block[i];
  }

  int blocks;
  int max = INT_MIN;
  int min = INT_MAX;
  int total_sum = 0;                  // Total number of records in all buckets
  int total_blocks = 0;               // Total number of blocks in all buckets
  int overflow_buckets = 0;          // Total number of buckets that have overflow blocks  
  BF_GetBlockCounter(desc, &blocks); // Get number of blocks in the file
  printf("Number of blocks: %d\n\n", blocks);

  for(int i = 0; i < buckets; i++){

    int sum = 0;                          // Calculate how many records we have 
    int last_block = hash_block[i];       // Last block of each bucket
    int records = (BF_BLOCK_SIZE - sizeof(HT_block_info)) / sizeof(Record);             // Total number of records in each block
    int bucket_blocks = 0;

    HT_block_info* block_info;

    // Traceback to the first block of the bucket , there is at least one block in every bucket from initialization
    do{
      // Get data from the latest block of the bucket
      CALL_OR_DIE(BF_GetBlock(desc, last_block, metadata));

      // Records array
      Record* cur_data = (Record*)BF_Block_GetData(metadata);    

      // Get metadata of that block
      void* addr = cur_data + records;
      block_info = (HT_block_info*)addr;

      // We are done with this one
      CALL_OR_DIE(BF_UnpinBlock(metadata));  

      // Access the previous block of the bucket
      last_block = block_info->overflow_block;

      // Update counter of how many records we've gone through
      sum += block_info->records ;
      total_blocks+=1;
      bucket_blocks++;
    } while(last_block != -1);

    total_sum += sum;

    if (sum > max)    // Find the number of max records
      max = sum;
    if(sum < min)     // Find the number of min records
      min = sum;

    if(bucket_blocks > 1) {             // Bucket blocks has the total number of blocks
      printf("Bucket number %d has %d overfow blocks\n",i , bucket_blocks-1);
      overflow_buckets++;
    }
    else {
      printf("Bucket number %d doesn't have overfow blocks\n",i);
    }
  }

  int avg_records = total_sum / buckets;      // Find the average number of records
  printf("Min number of records: %d\n", min);
  printf("Max number of records: %d\n", max);
  printf("Average number of records: %d\n", avg_records);

  int avg_blocks = total_blocks / buckets;    // Find the average number of records
  printf("Average number of blocks: %d\n", avg_blocks);
  printf(" There are %d buckets with overflow blocks." , overflow_buckets);

  BF_Block_Destroy(&metadata);
  return 0;
}