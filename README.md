# DBSM

## Heap

## Hash Table
Στο block 0 του αρχείου κρατάμε τα μεταδεδομένα στα οποία συμπεριλαμβάνεται ένας πίνακας που το index του αντιστοιχεί στον αριθμό του bucket. Σε κάθε θέση του πίνακα βρίσκεται ο αριθμός του τελευταίου block του αντίστοιχου bucket.
Επίσης στο struct HT_info κρατάμε μια πληροφορία ένα string το οποίο όταν έχει την τιμή "HashTable" γνωρίζουμε ότι η δομή πρόκειται όντως για HashTable. 
Τα μεταδεδόμενα κάθε block (HT_block_info) κρατάνε τον αριθμό του προηγούμενου block και αυτόν των τρέχουσων εγγραφών στο block.

(CreateFile) Όταν δημιουργούμε τα μεταδεδομένα δημιουργούμε και ένα αριθμό blocks αντίστοιχο στον αριθμό buckets που έχουμε ώστε να μπορεί να γίνει αντιστοίχιση στον πίνακα. Επειδή πρόκειται για τα αρχικά block και δεν έχουν προηγούμενο, η τιμή αυτή στα μεταδεδομένα θα είναι -1.
Δεν κάνουμε unpin το πρώτο block του αρχείου δίοτι κρατά τα μεταδεδομένα και θέλουμε να μείνει στη μνήμη. 

(HT_InsertEntry):
 Όταν κάνουμε μια εισαγωγή εγγραφής ελέγχουμε εαν το τελευταίο block χωράει αλλιώς δημιουργούμε νέο block και με τη βοήθεια της συνάρτησης `create_metadata` αρχικοποιούμε τα μεταδεδομένα του. Ανανεώνουμε τα μεταδεδομένα του αρχείου, αφού έχει αλλάξει το τελευταίο block του bucket και γράφουμε τις αλλαγές αυτές στο block 0. Αν το τελευταίο block χωράει κι άλλη εγγραφή τότε η εγγραφή εισάγεται σε αυτό. Τέλος, αυξάνουμε τον αριθμό των εγγραφών στο συγκεκριμένο block.

(CloseFile) : Εδώ γίνεται unpin το block 0.


(GetAllEntries): Hasharουμε το κλειδί που μας δίνεται από την συνάρτηση και προσπελαύνουμε τα blocks του bucket ξεκινώντας από το τελευταίο και πηγαίνοντας αναδρομικά προς τα πίσω στο πρώτο, ελέγχοντας τις εγγραφές του κάθε block.

(HashStatistics): Ακολουθούμε παρόμοια λογική με την συνάρτηση GetAllEntries, μόνο που τώρα αποκτάμε πρόσβαση στα μεταδεδομένα του αρχείου πρώτα μέσα από το block 0. Υπολογίζουμε το πόσα blocks έχει ένα αρχείο και από όλα τα buckets βρίσκουμε ποιο έχει το ελάχιστο, το μέσο και το μέγιστο πλήθος εγγραφών. Ακόμη, υπολογίζουμε το μέσο αριθμό των blocks που έχει κάθε bucket και το πλήθος των buckets που έχουν μπλοκ υπερχείλισης, και πόσα μπλοκ είναι αυτά για κάθε bucket όπως ζητείται.

Παρατηρούμε οτι με βάση το hash function που γίνεται με τον αύξοντα αριθμό του id, οι εγγραφές θα είναι ισομοιρασμένες στα buckets. Επομένως τρέχοντας το hash statistics περιμένουμε ίσο ελάχιστο, μέσο και μέγιστο αριθμό εγγραφών ανα block.