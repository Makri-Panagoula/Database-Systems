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

  //Finding the latest block of the bucket
  int bucket = hash_function(record.id, ht_info->buckets);
  int blocks = ht_info->hash_block[bucket];

  // Get data from that block 
  BF_Block* prev;
  BF_Block_Init(&prev);

  CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, blocks, prev));   
  Record* prev_data = (Record*) BF_Block_GetData(prev);       //SUS!

  // The available bytes for records per block will be its total size minus the pointer part (its metadata)
  // The number of records per block will be the available bytes divided by the size of each record (records have a fixed size)

  int tot_records = (BF_BLOCK_SIZE - sizeof(HT_block_info)) / sizeof(Record);

  //Get metadata of the latest block
  void* addr = prev_data + tot_records;
  HT_block_info* info = (HT_block_info*) addr;

  // If we have reached the number of records the block can store or we are in the first one 
  // that only stores the metadata, we allocate a new block, we store its pointer
  // to the metadata part of the previous one (the formerly last) and we place the record in a new block.

  if( info->records == tot_records || blocks == 1) {

    BF_Block* new;
    BF_Block_Init(&new);
    CALL_OR_DIE(BF_AllocateBlock(ht_info->fileDesc, new));

    //Update metadata of the hash table
    int last_block;
    CALL_OR_DIE(BF_GetBlockCounter(ht_info->fileDesc, &last_block));     
    ht_info->hash_block[bucket] = last_block ;

    // Put record in the new block + fix struct data
    Record* new_block = (Record*)BF_Block_GetData(new);
    new_block[0] = record;
    HT_block_info metadata;
    void* addr_new= new_block + tot_records;
    HT_block_info* addr_info = (HT_block_info*) addr_new;
    metadata.records = 1;
    memcpy(addr_info , &metadata ,sizeof(HT_block_info));

    // New block changed and now we are done with it 
    BF_Block_SetDirty(new); 
    CALL_OR_DIE(BF_UnpinBlock(new));      
    BF_Block_Destroy(&new);

    // Store pointer to new block as metadata
    void* addr;
    // The first block does not contain a pointer to the next block, only metadata of the heap file
    if(blocks != 1){   
      info->overflow_block = new;
    }
    else {
      // We keep the first block pinned until we close the file
      BF_Block_Destroy(&prev);
      return 0;
    }
  }
  // Store the record in the last block
  else {

    prev_data[info->records] = record;
    info->records++;
  }

  // Last block changed and now we are done with it 
  BF_Block_SetDirty(prev); 
  CALL_OR_DIE(BF_UnpinBlock(prev));  
  BF_Block_Destroy(&prev);

  return 0;
}
int HT_GetAllEntries(HT_info* ht_info, void *value ){
    return 0;
}




