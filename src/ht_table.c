#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "ht_table.h"
#include "record.h"

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }

int hash_function(int id , int buckets) {
  return id % buckets; 
}


int HT_CreateFile(char *fileName,  int buckets){

  BF_Block* block;
  BF_Block_Init(&block);

  //Create the hash table
  CALL_OR_DIE(BF_CreateFile(fileName));

  // Open the file to get access to it
  int desc;
  BF_OpenFile(fileName, &desc); 

  // Get the data of the block and write to it 
  // identifying it as a heap file
  CALL_OR_DIE(BF_AllocateBlock(desc, block)); 
  char* before = BF_Block_GetData(block);

  // Allocate memory address for the pointer 
  HT_info* info = malloc(sizeof(HT_info));

  // Initializing HT_INFO fields
  info->fileDesc = desc;
  info->buckets = buckets;    
  info->hash="HashTable";

  //We initially place the register that hashes to i inti the i-th block 
  for(int i=0; i<buckets; i++)
    info->hash_block[i]=i;

  memcpy(before, info, sizeof(HT_info));  

 // We have changed the content of the block, so we set it dirty to be recopied in hard disk
 // We don't unpin, we want to keep the first block with the metadata in the heap.
  BF_Block_SetDirty(block); 

  BF_Block_Destroy(&block);       // Take care of initialized block
  return 0;
}

HT_info* HT_OpenFile(char *fileName){
  // Open file 
  int desc;
  BF_OpenFile(fileName, &desc); 

  BF_Block* metadata;
  BF_Block_Init(&metadata);

  // Get data from first block
  BF_GetBlock(desc, 0, metadata); 
  HT_info* info = (HT_info*)BF_Block_GetData(metadata);

  // If the key holds an unexpected value, then this is not a heap file
  if( info == NULL || strcmp(info->hash,"HashTable") != 0)
    return NULL;  

  // Done with the block.Don't unpin!
  BF_Block_Destroy(&metadata);

  return info;  
}


int HT_CloseFile( HT_info* HT_info ){
  // Close file with the identical fileDesc
  BF_Block* metadata;
  BF_Block_Init(&metadata);
  BF_GetBlock(HT_info->fileDesc, 0, metadata);   

  CALL_OR_DIE(BF_UnpinBlock(metadata));
  BF_Block_Destroy(&metadata);

  CALL_OR_DIE(BF_CloseFile(HT_info->fileDesc));
  return 0;  
}

int HT_InsertEntry(HT_info* ht_info, Record record){

    return 0;
}

int HT_GetAllEntries(HT_info* ht_info, void *value ){
    return 0;
}




