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

// Initializes HP_block_info fields with number of records and previous_block  
void create_metadata(BF_Block* block, int number_of_records, int previous_block){

    Record* data = (Record*)BF_Block_GetData(block);  

    int records = (BF_BLOCK_SIZE - sizeof(HT_block_info)) / sizeof(Record);
    void* addr_new = data + records;
    HT_block_info* addr_info = (HT_block_info*) addr_new;
    HT_block_info metadata;

    metadata.records = number_of_records;
    metadata.overflow_block = previous_block;
    memcpy(addr_info, &metadata, sizeof(HT_block_info));
}

int HT_CreateFile(char *fileName,  int buckets){

  BF_Block* block;
  BF_Block_Init(&block);

  // Create the hash table
  CALL_OR_DIE(BF_CreateFile(fileName));

  // Open the file to get access to it
  int desc;
  BF_OpenFile(fileName, &desc); 

  // Get the data of the block and write to it 
  // identifying it as a heap file
  CALL_OR_DIE(BF_AllocateBlock(desc, block)); 
  char* before = BF_Block_GetData(block);

  // Initializing HT_INFO fields
  HT_info info;
  info.fileDesc = desc;
  info.buckets = buckets;    
  info.hash = "HashTable";

  // We initially place the register that hashes to i into the i-th block 
  for(int i = 0; i < buckets; i++){

    // We use the first block for metadata of the file so it can't be put in a bucket
    // Now every bucket has an initial block in consecutive order
    info.hash_block[i] = i + 1;
    CALL_OR_DIE(BF_AllocateBlock(desc, block));

    create_metadata(block, 0, -1);

    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));    
  }

  memcpy(before, &info, sizeof(HT_info));  

  // We have changed the content of the block, so we set it dirty to be recopied in hard disk
  // We don't unpin since we want to keep the first block with the metadata in the heap
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
  if(info == NULL || strcmp(info->hash,"HashTable") != 0)
    return NULL;  

  // Initialize the pointer with the struct values
  HT_info* to_return = malloc(sizeof(HT_info));
  memcpy(to_return, info, sizeof(HT_info));

  // Done with the block
  // Do not unpin!
  BF_Block_Destroy(&metadata);

  return to_return;  
}


int HT_CloseFile( HT_info* HT_info ){
  // Close file with the identical fileDesc
  BF_Block* metadata;
  BF_Block_Init(&metadata);
  BF_GetBlock(HT_info->fileDesc, 0, metadata);   

  CALL_OR_DIE(BF_UnpinBlock(metadata));
  BF_Block_Destroy(&metadata);

  CALL_OR_DIE(BF_CloseFile(HT_info->fileDesc));
  free(HT_info);
  return 0;  
}

int HT_InsertEntry(HT_info* ht_info, Record record){

  int bucket = hash_function(record.id, ht_info->buckets);
  // The last block of the bucket
  int blocks = ht_info->hash_block[bucket];

  // Get data from that block 
  BF_Block* prev;
  BF_Block_Init(&prev);
  CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, blocks, prev));   
  Record* prev_data = (Record*) BF_Block_GetData(prev);       // SUS!

  // The available bytes for records per block will be its total size minus the pointer part (its metadata)
  // The number of records per block will be the available bytes divided by the size of each record (records have a fixed size)
  int tot_records = (BF_BLOCK_SIZE - sizeof(HT_block_info)) / sizeof(Record);

  // Get metadata of that block
  void* addr = prev_data + tot_records;
  HT_block_info* info = (HT_block_info*)addr;

  // Count how many blocks in total are in the hash table
  int last_block;
  CALL_OR_DIE(BF_GetBlockCounter(ht_info->fileDesc, &last_block));

  // If we have reached the number of records the block can store or we are in the first one 
  // that only stores the metadata, we allocate a new block, we store its metadata,
  // the pointer to the previous one and we save the record in it

  if( info->records == tot_records || last_block == 1) {

    BF_Block* new;
    BF_Block_Init(&new);
    CALL_OR_DIE(BF_AllocateBlock(ht_info->fileDesc, new));

    // Update metadata of the hash table     
    ht_info->hash_block[bucket] = last_block;

    // Put record in the new block + fix metadata
    Record* new_block = (Record*)BF_Block_GetData(new);
    new_block[0] = record;

    create_metadata(new, 1, blocks);

    // New block changed and now we are done with it 
    BF_Block_SetDirty(new); 
    CALL_OR_DIE(BF_UnpinBlock(new));      
    BF_Block_Destroy(&new);

    if(last_block == 1) {
      // We keep the first block pinned until we close the file
      BF_Block_Destroy(&prev);
      return ht_info->hash_block[bucket];
    }
  }
  // Store the record in the previous, already existent block
  else {
    prev_data[info->records] = record;
    info->records++;
  }

  // Last block changed and now we are done with it 
  BF_Block_SetDirty(prev); 
  CALL_OR_DIE(BF_UnpinBlock(prev));  
  BF_Block_Destroy(&prev);

  return ht_info->hash_block[bucket];
}


int HT_GetAllEntries(HT_info* ht_info, void *value ){

  int id = * (int*) value;

  // Finding the latest block of the bucket
  int bucket = hash_function(id, ht_info->buckets);
  int blocks = ht_info->hash_block[bucket];
  int counter = 0;      // Count how many blocks we will access.
  BF_Block* cur;
  BF_Block_Init(&cur);

  // Get data from the latest block of the bucket
  CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, blocks, cur));

  // Records array
  Record* cur_data = (Record*)BF_Block_GetData(cur);    

  // Total number of records in each block
  int records = (BF_BLOCK_SIZE - sizeof(HT_block_info)) / sizeof(Record);

  // Get metadata of that block
  void* addr = cur_data + records;
  HT_block_info* info = (HT_block_info*)addr;


  // Traceback to the first block of the bucket
  while(info->overflow_block != -1){

    // Check every record of the block
    for(int i = 0; i < records; i++){

      if(cur_data[i].id == id)
        printRecord(cur_data[i]);
    }

    // We are done with this one
    CALL_OR_DIE(BF_UnpinBlock(cur));  

    // Access the previous block of the bucket
    blocks = info->overflow_block;
    CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, blocks, cur));
    cur_data = (Record*)BF_Block_GetData(cur);

    // Get metadata of that block
    addr = cur_data + records;
    info = (HT_block_info*)addr;
    // Update counter
    counter++;
  }

  BF_Block_Destroy(&cur);
  
  return counter;
}




