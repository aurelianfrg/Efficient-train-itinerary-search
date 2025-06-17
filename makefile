# ================= Options de compilation =================
GCC = g++
CCFLAGS = -ansi -pedantic -Wall -std=c++17 -O2 #-g -D MAP
#LIBS = -lm

# ================= Localisations =================
SRC_PATH = src
INT_PATH = src
OBJ_PATH = obj
BIN_PATH = bin
MAIN = main
EXE = main

# ================= Options du clean =================
CLEAN = clean
RM = rm
RMFLAGS = -f
.PHONY: $(CLEAN)

OTHER = nothing
.PHONY: $(OTHER)

# ===============================================
# ================= COMPILATION =================
# ===============================================

# Compilation des exécutables finaux
$(EXE): $(OBJ_PATH)/$(MAIN).o $(OBJ_PATH)/GTFSDataParser.o $(OBJ_PATH)/Trajet.o
	@mkdir -p $(BIN_PATH)
	$(GCC) -o $(BIN_PATH)/$@ $^ $(LIBS)

parsing: $(OBJ_PATH)/parsing.o
	@mkdir -p $(BIN_PATH)
	$(GCC) -o $(BIN_PATH)/$@ $^ $(LIBS)

server: $(OBJ_PATH)/server.o $(OBJ_PATH)/GTFSDataParser.o $(OBJ_PATH)/Trajet.o
	@mkdir -p $(BIN_PATH)
	$(GCC) -o $(BIN_PATH)/$@ $^ $(LIBS)

# Compilation des fichiers objets
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp
	@mkdir -p $(OBJ_PATH)
	$(GCC) $(CCFLAGS) -c $< -o $@

# ================= Clean =================
$(CLEAN):
	$(RM) $(RMFLAGS) $(OBJ_PATH)/*.o $(BIN_PATH)/*

run: $(BIN_PATH)/server
	
