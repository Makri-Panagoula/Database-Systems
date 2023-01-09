# DBSM

Ιωάννα Κοντεμενιώτου - Α.Μ: 1115202000227 <br/>
Παναγούλα Μακρή -  Α.Μ: 115202000119

## Heap
Ως αναγνωριστίκο στο struct HP_info έχουμε ένα string το οποίο έχει την τιμή "Heap" για να γνωρίζουμε ότι η δομή πρόκειται όντως για Heap. Δεν κάνουμε unpin το 1ο block παρα μόνο στην συνάρτηση `Closefile`. Εισάγουμε δεδομένα στο τέλος του αρχείου και αν δεν χωράνε στο ήδη υπάρχον block δημιουργούμε νέο. 

**HP_GetAllEntries:** Προσπελαύνουμε όλες τις εγγραφές σειριακά 


## Hash Table
Στο block 0 του αρχείου κρατάμε τα μεταδεδομένα στα οποία συμπεριλαμβάνεται ένας πίνακας που το index του αντιστοιχεί στον αριθμό του bucket. Σε κάθε θέση του πίνακα βρίσκεται ο αριθμός του τελευταίου block του αντίστοιχου bucket.
Επίσης στο struct HT_info κρατάμε μια πληροφορία ένα string το οποίο όταν έχει την τιμή "HashTable" για να γνωρίζουμε ότι η δομή πρόκειται όντως για HashTable. 
Τα μεταδεδόμενα κάθε block (HT_block_info) κρατάνε τον αριθμό του προηγούμενου block και αυτόν των τρέχουσων εγγραφών στο block.

**HT_CreateFile:** Όταν δημιουργούμε τα μεταδεδομένα δημιουργούμε και ένα αριθμό blocks αντίστοιχο στον αριθμό buckets που έχουμε ώστε να μπορεί να γίνει αντιστοίχιση στον πίνακα. Επειδή πρόκειται για τα αρχικά block και δεν έχουν προηγούμενο, η τιμή αυτή στα μεταδεδομένα θα είναι -1.
Δεν κάνουμε unpin το πρώτο block του αρχείου δίοτι κρατά τα μεταδεδομένα και θέλουμε να μείνει στη μνήμη. 

**HT_InsertEntry:**
Όταν κάνουμε μια εισαγωγή εγγραφής ελέγχουμε εαν το τελευταίο block χωράει αλλιώς δημιουργούμε νέο block και με τη βοήθεια της συνάρτησης `create_metadata` αρχικοποιούμε τα μεταδεδομένα του. Ανανεώνουμε τα μεταδεδομένα του αρχείου, αφού έχει αλλάξει το τελευταίο block του bucket και γράφουμε τις αλλαγές αυτές στο block 0. Αν το τελευταίο block χωράει κι άλλη εγγραφή τότε η εγγραφή εισάγεται σε αυτό. Τέλος, αυξάνουμε τον αριθμό των εγγραφών στο συγκεκριμένο block.

**HT_CloseFile:** Εδώ γίνεται unpin το block 0.


**HT_GetAllEntries:** Hasharουμε το κλειδί που μας δίνεται από την συνάρτηση και προσπελαύνουμε τα blocks του bucket ξεκινώντας από το τελευταίο και πηγαίνοντας αναδρομικά προς τα πίσω στο πρώτο, ελέγχοντας τις εγγραφές του κάθε block. Επιστρέφει τον αριθμο blocks που έχουμε προσπελάσει.

**HashStatistics:** Ακολουθούμε παρόμοια λογική με την συνάρτηση `HT_GetAllEntries`, μόνο που τώρα αποκτάμε πρόσβαση στα μεταδεδομένα του αρχείου πρώτα μέσα από το block 0. Υπολογίζουμε το πόσα blocks έχει ένα αρχείο και από όλα τα buckets βρίσκουμε ποιο έχει το ελάχιστο, το μέσο και το μέγιστο πλήθος εγγραφών. Ακόμη, υπολογίζουμε το μέσο αριθμό των blocks που έχει κάθε bucket και το πλήθος των buckets που έχουν μπλοκ υπερχείλισης, και πόσα μπλοκ είναι αυτά για κάθε bucket όπως ζητείται.

Παρατηρούμε οτι με βάση το hash function που γίνεται με τον αύξοντα αριθμό του id, οι εγγραφές θα είναι ισομοιρασμένες στα buckets. Επομένως τρέχοντας το `HashStatistics` περιμένουμε ίσο ελάχιστο, μέσο και μέγιστο αριθμό εγγραφών ανα block.

## Makefile
Use the Makefile to compile, run and clean using the following commands:

**Heap** <br/>

```bash
$ make hp
$ make runhp
$ make clean
```

**HashTable** <br/>

```bash
$ make ht
$ make runht
$ make clean
```

## Secondary Hash Table
Ως αναγνωριστίκο στο struct SHT_info έχουμε ένα string το οποίο έχει την τιμή "Secondary HashTable" για να γνωρίζουμε ότι η δομή πρόκειται όντως για Secondary Hash Table. Δεν κάνουμε unpin το 1ο block παρα μόνο στην συνάρτηση `Closefile`. Εισάγουμε δεδομένα στο τέλος του αρχείου και αν δεν χωράνε στο ήδη υπάρχον block δημιουργούμε νέο. Άκολουθούμε την ίδια υλοποιήση με το Hash Table με διαφορά ότι εισάγουμε ζεύγος που αποτελείται από το όνομα της κάθε εγγραφής και τον αριθμό του block που βρίσκεται στο Hash Table.

**sht_metadata**
Καλούμε τη συνάρτηση αυτή κάθε φορά που θέλουμε δημιουργούμε ένα νέο block σε κάθε bucket και αρχικοποιούμε τις μεταβλητές σε ένα SHT_block_info.

**SHT_OpenSecondaryIndex**
Αποκτάμε πρόσβαση στο πρώτο block με τα μεταδεδομένα του αρχείου και τα αντιγράφουμε σε έναν άλλο δείκτη τον οποίο και επιστρέφουμε.

**SHT_SecondaryGetAllEntries:** Προσπελαύνουμε όλες τις εγγραφές σειριακά 

Στο block 0 του αρχείου κρατάμε τα μεταδεδομένα στα οποία συμπεριλαμβάνεται ένας πίνακας που το index του αντιστοιχεί στον αριθμό του bucket. Σε κάθε θέση του πίνακα βρίσκεται ο αριθμός του τελευταίου block του αντίστοιχου bucket.
Επίσης στο struct HT_info κρατάμε μια πληροφορία ένα string το οποίο όταν έχει την τιμή "HashTable" για να γνωρίζουμε ότι η δομή πρόκειται όντως για HashTable. 
Τα μεταδεδόμενα κάθε block (HT_block_info) κρατάνε τον αριθμό του προηγούμενου block και αυτόν των τρέχουσων εγγραφών στο block.

**SHT_CreateSecondaryIndex:** Όταν δημιουργούμε τα μεταδεδομένα δημιουργούμε και ένα αριθμό blocks αντίστοιχο στον αριθμό buckets που έχουμε ώστε να μπορεί να γίνει αντιστοίχιση στον πίνακα. Επειδή πρόκειται για τα αρχικά block και δεν έχουν προηγούμενο, η τιμή αυτή στα μεταδεδομένα θα είναι -1. Δεν κάνουμε unpin το πρώτο block του αρχείου δίοτι κρατά τα μεταδεδομένα και θέλουμε να μείνει στη μνήμη. 

**SHT_SecondaryInsertEntry:**
Όταν κάνουμε μια εισαγωγή εγγραφής ελέγχουμε εαν το τελευταίο block χωράει αλλιώς δημιουργούμε νέο block και με τη βοήθεια της συνάρτησης `sht_metadata` αρχικοποιούμε τα μεταδεδομένα του. Ανανεώνουμε τα μεταδεδομένα του αρχείου, αφού έχει αλλάξει το τελευταίο block του bucket και γράφουμε τις αλλαγές αυτές στο block 0. Αν το τελευταίο block χωράει κι άλλη εγγραφή τότε η εγγραφή εισάγεται σε αυτό. Τέλος, αυξάνουμε τον αριθμό των εγγραφών στο συγκεκριμένο block.

**SHT_CloseSecondaryIndex:** Εδώ γίνεται unpin το block 0.

**SHT_SecondaryGetAllEntries:** Hasharουμε το κλειδί που μας δίνεται από την συνάρτηση (name) και προσπελαύνουμε τα blocks του bucket ξεκινώντας από το τελευταίο και πηγαίνοντας αναδρομικά προς τα πίσω στο πρώτο, ελέγχοντας τις εγγραφές του κάθε block. Αφού βρούμε το ζητούμενο όνομα, έχουμε πρόσβαση στο block της εγγραφής στο HashTable και έπειτα προσπελαύνουμε όλες τις εγγραφές του για να εντοπίσουμε ολόκληρη την εγγραφή. Επιστρέφει τον αριθμο blocks που έχουμε προσπελάσει.

**HashStatistics:** Ακολουθούμε παρόμοια λογική με την συνάρτηση `SHT_SecondaryGetAllEntries`, μόνο που τώρα αποκτάμε πρόσβαση στα μεταδεδομένα του αρχείου πρώτα μέσα από το block 0. Υπολογίζουμε το πόσα blocks έχει ένα αρχείο και από όλα τα buckets βρίσκουμε ποιο έχει το ελάχιστο, το μέσο και το μέγιστο πλήθος εγγραφών. Ακόμη, υπολογίζουμε το μέσο αριθμό των blocks που έχει κάθε bucket και το πλήθος των buckets που έχουν μπλοκ υπερχείλισης, και πόσα μπλοκ είναι αυτά για κάθε bucket όπως ζητείται.

Παρατηρούμε οτι με βάση το hash function που γίνεται με τον αύξοντα αριθμό του id, οι εγγραφές θα είναι ισομοιρασμένες στα buckets. Επομένως τρέχοντας το `HashStatistics` περιμένουμε ίσο ελάχιστο, μέσο και μέγιστο αριθμό εγγραφών ανα block για το hashtable.

Επειδή διαφέρουν oι δομές και τα πεδία τους καλούμε διαφορετική συνάρτηση ανάλογα με το για ποια δομή ενδιαφερόμαστε για να είναι πιο καθαρός ο κώδικας.Το σκεπτικό όμως παραμένει ακριβώς ίδιο.

## Makefile
Use the Makefile to compile, run and clean using the following commands:

**Secondary Hash Table** <br/>

```bash
$ make sht
$ make runsht
$ make clean
```