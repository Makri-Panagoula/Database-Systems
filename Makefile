# Paths
INCLUDE = ./include/
LIBRARY = ./lib/
BUILD = ./build/
SRC = ./src/

DB = *.db

# Object Files
OBJ = $(BUILD)bf_main $(BUILD)hp_main

hp:
	@echo " Compile hp_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/hp_main.c $(SRC)record.c $(SRC)hp_file.c -lbf -o $(BUILD)hp_main -O2 -g

bf:
	@echo " Compile bf_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/bf_main.c $(SRC)record.c -lbf -o $(BUILD)bf_main -O2 ;

ht:
	@echo " Compile ht_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/ht_main.c $(SRC)record.c $(SRC)ht_table.c -lbf -o $(BUILD)ht_main -O2 -g 


sht:
	@echo " Compile hp_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/sht_main.c $(SRC)record.c $(SRC)sht_table.c $(SRC)ht_table.c -lbf -o $(BUILD)sht_main -O2

runhp:
	@echo "Runing hp:"
	$(BUILD)hp_main

runbf:
	@echo "Running bf:"
	$(BUILD)bf_main

runht:
	@echo "Running ht:"
	$(BUILD)ht_main

runsht: 
	@echo "Running sht:"
	$(BUILD)sht_main


# Clean
clean: 
	@echo "Removing previous db files..."
	rm -f $(DB)
	@echo "Cleaning..."
	rm -f $(OBJ)
