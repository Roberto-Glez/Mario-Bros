# Directorios
SRC_DIR := src
BIN_DIR := bin
INC_DIR := include

# Librerías (IMPORTANTE: Esto asume que están instaladas en tu sistema)
LIBS := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d

# Archivos
CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
HPP_FILES := $(wildcard $(INC_DIR)/*.hpp)
EXE_FILE := $(BIN_DIR)/mario_bros.exe

# Compilador
CXX := g++
CXXFLAGS := -I$(INC_DIR) -Wall -std=c++17

# Regla principal (el "Target" por defecto)
all: $(EXE_FILE)

# Regla para compilar
$(EXE_FILE): $(CPP_FILES) $(HPP_FILES)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CPP_FILES) -o $@ $(CXXFLAGS) $(LIBS)

# Regla para limpiar
clean:
	rm -f $(BIN_DIR)/*.exe