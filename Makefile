# Paths
INCLUDE = ./include/
LIBRARY = ./lib/
BUILD = ./build/

DB = *.db

# Object Files
OBJ = $(BUILD)bf_main $(BUILD)hp_main

hp:
	@echo " Compile hp_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/hp_main.c ./src/record.c ./src/hp_file.c -lbf -o $(BUILD)hp_main -O2

bf:
	@echo " Compile bf_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/bf_main.c ./src/record.c -lbf -o $(BUILD)bf_main -O2;

ht:
	@echo " Compile ht_main ...";
	gcc -I $(INCLUDE) -L $(LIBRARY) -Wl,-rpath,$(LIBRARY) ./examples/ht_main.c ./src/record.c ./src/ht_table.c -lbf -o $(BUILD)ht_main -O2



runhp:
	@echo "Runing hp:"
	$(BUILD)hp_main

runbf:
	@echo "Running bf:"
	$(BUILD)bf_main

runht:
	@echo "Running ht:"
	$(BUILD)ht_main


# Clean
clean: 
	@echo "Removing previous db files..."
	rm -f $(DB)
	@echo "Cleaning..."
	rm -f $(OBJ)