#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bf.h"
#include "ht_table.h"
#include "sht_table.h"
//#include "hash_statistics.h"

#define RECORDS_NUM 350 // you can change it if you want
#define FILE_NAME "data.db"   // Hash Table
#define INDEX_NAME "index.db" // Secondary Hash Table

#define CALL_OR_DIE(call)     \
  {                           \
    BF_ErrorCode code = call; \
    if (code != BF_OK) {      \
      BF_PrintError(code);    \
      exit(code);             \
    }                         \
  }


int main() {
    srand(12569874);
    BF_Init(LRU);

    /* Create Hash File */
    if(HT_CreateFile(FILE_NAME, 10) != 0)
      printf("Error! Could not create file\n");

    /* Create Secondary Hash File */
    if(SHT_CreateSecondaryIndex(INDEX_NAME, 10, FILE_NAME) != 0)
      printf("Error! Could not create file\n");
    
    HT_info* info = HT_OpenFile(FILE_NAME); 
    if(info == NULL)
      printf("Error! Could not open file\n");

    SHT_info* index_info = SHT_OpenSecondaryIndex(INDEX_NAME);
    if (index_info == NULL)
      printf("Error! Could not open file\n");
    

    // Θα ψάξουμε στην συνέχεια το όνομα searchName
    Record record = randomRecord();
    char searchName[15];
    strcpy(searchName, record.name);

    // Κάνουμε εισαγωγή τυχαίων εγγραφών τόσο στο αρχείο κατακερματισμού τις οποίες προσθέτουμε και στο δευτερεύον ευρετήριο
    printf("---------- Insert Entries ----------\n");
    for (int id = 0; id < RECORDS_NUM; ++id) {
        record = randomRecord();
        int block_id = HT_InsertEntry(info, record);
        if(SHT_SecondaryInsertEntry(index_info, record, block_id) == -1)
          printf("Error in Insert!");
    }

    // Τυπώνουμε όλες τις εγγραφές με όνομα searchName
    printf("RUN PrintAllEntries for name %s\n", searchName);
    if(SHT_SecondaryGetAllEntries(info, index_info, searchName) == 0)
      printf("Error in SHT_SecondaryGetAllEntries!\n");

    // Testing SHTStatistics
    printf("\n---------- STATISTICS ----------\n");
    if(HashStatistics(INDEX_NAME) != 0) 
      printf("Error in HashStatistics\n");

    // Κλείνουμε το αρχείο κατακερματισμού και το δευτερεύον ευρετήριο
    if(SHT_CloseSecondaryIndex(index_info) == 0)
      printf("Closed file successfully!\n");
    else printf("Error! Could not close file.\n");
    
    HT_CloseFile(info);
    BF_Close();
}

