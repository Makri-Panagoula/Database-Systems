#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bf.h"
#include "hp_file.h"
#define MAX_OPEN_FILES 20
#define HP_ERROR -1;

#define CALL_BF(call)       \
{                           \
  BF_ErrorCode code = call; \
  if (code != BF_OK) {         \
    BF_PrintError(code);    \
    return HP_ERROR;        \
  }                         \
}

int HP_CreateFile(char *fileName){
  // Create file
  CALL_BF(BF_CreateFile(fileName));

  // Open file
  int desc;
  BF_OpenFile(fileName, &desc); 

  // Create a block in the file 
  BF_Block* block;
  BF_Block_Init(&block);

  // Allocate memory
  CALL_BF(BF_AllocateBlock(desc, block)); 

  char* before = BF_Block_GetData(block);     // Get data of the first block
  memcpy(before, "Heap", strlen("Heap")+1);   // Save some arbitrary metadata in it

  BF_Block_SetDirty(block); 
  CALL_BF(BF_UnpinBlock(block));

  // Close file
  BF_Block_Destroy(&block); 
  CALL_BF(BF_CloseFile(desc));

  return 0;
}

// Κραταει και την πληροφορια οτι το αρχειο ειναι οντως αρχειο σωρου
HP_info* HP_OpenFile(char *fileName){
  // Open file 
  int desc;
  BF_OpenFile(fileName, &desc); 

  BF_Block* metadata;
  BF_Block_Init(&metadata);

  // Get data from first block
  BF_GetBlock(desc, 0, metadata); 
  char* heap = (char*)BF_Block_GetData(metadata);

  // If the key holds an unexpected value, then this is not a heap file 
  if(strcmp(heap,"Heap") != 0)
    return NULL;  

  HP_info* info = malloc(sizeof(HP_info));
  info->fileDesc = desc;
  info->bytes = 0;

  // We don't need these
  // BF_Block_Destroy(&metadata);
  // BF_CloseFile(desc);
  
  return info;
}


int HP_CloseFile(HP_info* header_info ){
  // Close file with the identical fileDesc
  CALL_BF(BF_CloseFile(header_info->fileDesc));
  return 0;
}

//Σε καθε block πρέπει να κραταμε εναν δεικτη που θα μας πηγαίνει στο επόμενο block
//Στα μεταδεδομενα κραταμε που ακριβως θα γινει η εισαγωγη

int HP_InsertEntry(HP_info* header_info, Record record){
  int bytes_left = BF_BLOCK_SIZE - header_info->bytes - sizeof(BF_Block*);
  int to_add = sizeof(record);

  BF_Block* prev;
  BF_Block_Init(&prev);

  int blocks;
  CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &blocks));
  //printf("Number of blocks: %d\n",blocks);

  // Getting data from the last block 
  CALL_BF(BF_GetBlock(header_info->fileDesc, blocks -1, prev));   
  char* prev_data = BF_Block_GetData(prev);

  // Remeber!! Block enumeration starts from 0

  // If there is not enough space to put the new record in
  if(to_add > bytes_left || blocks == 1) {
    // Create a new block 
    BF_Block* new;
    BF_Block_Init(&new);
    CALL_BF(BF_AllocateBlock(header_info->fileDesc,new));

    // Store pointer to new block as metadata
    memcpy(prev_data + header_info->bytes, new, sizeof(BF_Block*));

    // Put record in the new block
    char* new_block = BF_Block_GetData(new);
    memcpy(new_block , &record , sizeof(Record));
    header_info->bytes = sizeof(Record);  

    // Take care of the block
    BF_Block_SetDirty(new); 
    CALL_BF(BF_UnpinBlock(new));      
    BF_Block_Destroy(&new);
  }
  // If we do not need to create a new block
  else {
    // Store the record in the last available block
    memcpy(prev_data + header_info->bytes , &record , sizeof(Record));    
    header_info->bytes+=sizeof(Record);
  }

  BF_Block_SetDirty(prev); 
  CALL_BF(BF_UnpinBlock(prev));  

  BF_Block_Destroy(&prev);
  return 0;
}

int HP_GetAllEntries(HP_info* header_info, int id ){

  BF_Block* cur;
  BF_Block_Init(&cur);
  printf("\ninitialization done\n");
  int blocks;
  CALL_BF(BF_GetBlockCounter(header_info->fileDesc, &blocks));
  printf("\ncounting blocks %d\n", blocks);

  int records_per_block = (BF_BLOCK_SIZE - sizeof(BF_Block*)) / sizeof(Record);

  if(blocks > BF_BUFFER_SIZE) {
    blocks = BF_BUFFER_SIZE;
  }

  for(int i=0; i < blocks; i++) {

    //Getting data from each block 
    CALL_BF(BF_GetBlock(header_info->fileDesc , i , cur));   
    printf("\n%d",i);
  
    char* cur_data = BF_Block_GetData(cur);   

    for(int j=0; j < records_per_block; j++) {        //For each record inside the block

      Record* rec= (Record*) (cur_data + j * sizeof(Record));

      if(rec->id == id) {
        printRecord(*rec);
      }
    } 
  }

//  CALL_BF(BF_UnpinBlock(cur));  
  BF_Block_Destroy(&cur);
  return blocks;
}

