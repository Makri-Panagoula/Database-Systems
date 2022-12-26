#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "sht_table.h"
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

void sht_metadata(BF_Block* block, int number_of_records, int previous_block){

  SHT_Record* data = (SHT_Record*)BF_Block_GetData(block);  
  // Number or records in every block
  int records = (BF_BLOCK_SIZE - sizeof(SHT_block_info)) / sizeof(SHT_Record);

  // Copy the metadata to the block once you initialize it
  void* addr_new = data + records;
  SHT_block_info* addr_info = (SHT_block_info*) addr_new;
  SHT_block_info metadata;

  metadata.records = number_of_records;
  metadata.overflow_block = previous_block;
  memcpy(addr_info, &metadata, sizeof(SHT_block_info));
}

int sht_hash_function(char name[MAX_BYTES] , int buckets) {
  
  int sum = 0;
  for(int i = 0; i < MAX_BYTES; i++){

    if(name[i] == '\0')    // End of the string
      break;

    sum += (int)name[i];   // Add the ASCII code of each character
  }
  
  int hash = sum % buckets;
  return hash;
}

void access_hashtable_block(HT_info* ht_info, int wanted_block , char* name) {
  BF_Block* block;
  BF_Block_Init(&block);

  CALL_OR_DIE(BF_GetBlock(ht_info->fileDesc, wanted_block, block));

  // Records array
  Record* cur_data = (Record*)BF_Block_GetData(block);    

  // Total number of records in each block
  int records = (BF_BLOCK_SIZE - sizeof(HT_block_info)) / sizeof(Record);

  // Get metadata of that block
  void* addr = cur_data + records;
  HT_block_info* info = (HT_block_info*)addr;

  // Check every record of the block
  for(int i = 0; i < info->records; i++){

    if(strcmp(cur_data[i].name, name) == 0)
      printRecord(cur_data[i]);
  }
  // We are done with this one
  CALL_OR_DIE(BF_UnpinBlock(block));  
  BF_Block_Destroy(&block);       // Take care of initialized block
}


int SHT_CreateSecondaryIndex(char *sfileName, int buckets, char* fileName){

  BF_Block* block;
  BF_Block_Init(&block);

  // Create the secondary hash table
  CALL_OR_DIE(BF_CreateFile(sfileName));

  // Open the secondary file to get access to it
  int sdesc;
  BF_OpenFile(sfileName, &sdesc); 

  // Open the file of hashtable to get access to it
  int fdesc;
  BF_OpenFile(sfileName, &fdesc); 

  // Get the data of the block and write to it 
  // identifying it as a secondary hash table file
  CALL_OR_DIE(BF_AllocateBlock(sdesc, block)); 
  char* before = BF_Block_GetData(block);

  // Initializing HT_INFO fields
  SHT_info info;
  info.sfileDesc = sdesc;
  info.fileDesc = fdesc;
  info.buckets = buckets;    
  info.identifier = "Secondary HashTable";

  // We initially place the register that hashes to i into the i-th block 
  for(int i = 0; i < buckets; i++){

    // We use the first block for metadata of the file so it can't be put in a bucket
    // Now every bucket has an initial block in consecutive order
    info.hash_block[i] = i + 1;
    CALL_OR_DIE(BF_AllocateBlock(sdesc, block));

    sht_metadata(block, 0, -1);

    BF_Block_SetDirty(block);
    CALL_OR_DIE(BF_UnpinBlock(block));    
  }

  memcpy(before, &info, sizeof(SHT_info));  

  // We have changed the content of the block, so we set it dirty to be recopied in hard disk
  // We don't unpin since we want to keep the first block with the metadata in the heap
  BF_Block_SetDirty(block); 

  BF_Block_Destroy(&block);       // Take care of initialized block
  return 0;
}


SHT_info* SHT_OpenSecondaryIndex(char *indexName){

  // Open file 
  int desc;
  CALL_OR_DIE(BF_OpenFile(indexName, &desc)); 

  BF_Block* metadata;
  BF_Block_Init(&metadata);

  // Get data from first block
  CALL_OR_DIE(BF_GetBlock(desc, 0, metadata)); 
  SHT_info* info = (SHT_info*)BF_Block_GetData(metadata);

  // If the key holds an unexpected value, then this is not a secondary hash table file
  if(info == NULL || strcmp(info->identifier,"Secondary HashTable") != 0)
    return NULL;  

  // Initialize the pointer with the struct values
  SHT_info* to_return = malloc(sizeof(SHT_info));
  memcpy(to_return, info, sizeof(SHT_info));

  // Done with the block
  // Do not unpin!
  BF_Block_Destroy(&metadata);

  return to_return;  
}


int SHT_CloseSecondaryIndex(SHT_info* SHT_info ){
  
  // Close file with the identical sfileDesc
  BF_Block* metadata;
  BF_Block_Init(&metadata);
  BF_GetBlock(SHT_info->sfileDesc, 0, metadata);   

  CALL_OR_DIE(BF_UnpinBlock(metadata));
  BF_Block_Destroy(&metadata);

  CALL_OR_DIE(BF_CloseFile(SHT_info->sfileDesc));
  CALL_OR_DIE(BF_CloseFile(SHT_info->fileDesc));

  free(SHT_info);
  return 0;  
}

int SHT_SecondaryInsertEntry(SHT_info* sht_info, Record record, int block_id){
  
  int bucket = sht_hash_function(record.name, sht_info->buckets);
  // Get the last block of the bucket
  int blocks = sht_info->hash_block[bucket];
  
  // Initialize entry
  SHT_Record sht_record;
  sht_record.block_id = block_id;
  strcpy(sht_record.name,record.name);

  // Get data from that block 
  BF_Block* prev;
  BF_Block_Init(&prev);
  CALL_OR_DIE(BF_GetBlock(sht_info->sfileDesc, blocks, prev));   
  SHT_Record* prev_data = (SHT_Record*) BF_Block_GetData(prev);       

  // The available bytes for records per block will be its total size minus the pointer part (its metadata)
  // The number of records per block will be the available bytes divided by the size of each record (records have a fixed size)
  int tot_records = (BF_BLOCK_SIZE - sizeof(SHT_block_info)) / sizeof(SHT_Record);

  // Get metadata of that block
  void* addr = prev_data + tot_records;
  SHT_block_info* info = (SHT_block_info*)addr;

  // Count how many blocks in total are in the hash table
  int last_block;
  CALL_OR_DIE(BF_GetBlockCounter(sht_info->sfileDesc, &last_block));

  // If we have reached the number of records the block can store or we are in the first one 
  // that only stores the metadata, we allocate a new block, we store its metadata,
  // the pointer to the previous one and we save the record in it

  if(info->records == tot_records || last_block == 1) {
    BF_Block* new;
    BF_Block_Init(&new);
    CALL_OR_DIE(BF_AllocateBlock(sht_info->sfileDesc, new));

    // Update metadata of the file and write the changes in block 0
    BF_Block* metadata;
    BF_Block_Init(&metadata);    
    CALL_OR_DIE(BF_GetBlock(sht_info->sfileDesc, 0 , metadata));   
    SHT_info* file_data = (SHT_info*) BF_Block_GetData(metadata);   
    sht_info->hash_block[bucket] = last_block;
    memcpy(file_data,sht_info,sizeof(sht_info));
    BF_Block_SetDirty(metadata);
    BF_Block_Destroy(&metadata);

    // Put SHT_Record in the new block + fix metadata
    SHT_Record* new_block = (SHT_Record*)BF_Block_GetData(new);
    new_block[0] = sht_record;

    sht_metadata(new,1,blocks);

    // New block changed and now we are done with it 
    BF_Block_SetDirty(new); 
    CALL_OR_DIE(BF_UnpinBlock(new));      
    BF_Block_Destroy(&new);

    if(last_block == 1) {
      // We keep the first block pinned until we close the file
      BF_Block_Destroy(&prev);
      return sht_info->hash_block[bucket];
    }
  }
  // Store the record in the previous, already existent block
  else {
    prev_data[info->records] = sht_record;
    info->records++;
  }

  // Last block changed and now we are done with it 
  BF_Block_SetDirty(prev); 
  CALL_OR_DIE(BF_UnpinBlock(prev));  
  BF_Block_Destroy(&prev);

  return sht_info->hash_block[bucket];
}

int SHT_SecondaryGetAllEntries(HT_info* ht_info, SHT_info* sht_info, char* name){
  
  // Finding the latest block of the bucket
  int bucket = sht_hash_function(name, sht_info->buckets);
  
  int blocks = sht_info->hash_block[bucket];
  int counter = 0;        // Count how many blocks we will access.
  BF_Block* cur;
  BF_Block_Init(&cur);
  SHT_block_info* info;
  int wanted_block;       // The corresponding block of the record in the hashtable

  // Traceback to the first block of the bucket
  do{
    // Get data from the latest block of the bucket
    CALL_OR_DIE(BF_GetBlock(sht_info->sfileDesc, blocks, cur));

    // Records array
    SHT_Record* cur_data = (SHT_Record*)BF_Block_GetData(cur);    

    // Total number of records in each block
    int records = (BF_BLOCK_SIZE - sizeof(SHT_block_info)) / sizeof(SHT_Record);

    // Get metadata of that block
    void* addr = cur_data + records;
    info = (SHT_block_info*)addr;

    // Check every record of the block
    for(int i = 0; i < info->records; i++){

      // If the name of the record we are accessing is the same as wanted,
      // we have found the block_id of the hashtable
      if(strcmp(cur_data[i].name, name) == 0) {

        wanted_block = cur_data[i].block_id;
        access_hashtable_block(ht_info, wanted_block, name);
      }
    }
    // We are done with this one
    CALL_OR_DIE(BF_UnpinBlock(cur));  

    // Access the previous block of the bucket
    blocks = info->overflow_block;

    // Update counter of how many blocks we've gone through
    counter++;

  }while(blocks != -1);

  BF_Block_Destroy(&cur);
  return counter;
}


