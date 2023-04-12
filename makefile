LFLAGS = -I./include/ -static-libgcc -static-libstdc++
FLAGS = -l:libboost_system.a -l:libboost_filesystem.a -l:libboost_thread.a -l:libboost_chrono.a -l:libboost_serialization.a -lpthread
WX_FLAGS = `wx-config --cxxflags --libs std`

SRC_DIR := ./src
OBJ_DIR := ./build/obj
EXE_DIR := ./build/executables

SOURCES := $(wildcard $(SRC_DIR)/filesystem/*.cpp) $(wildcard $(SRC_DIR)/Log/*.cpp) $(wildcard $(SRC_DIR)/misc/*.cpp) $(wildcard $(SRC_DIR)/network/*.cpp) $(wildcard $(SRC_DIR)/P2P/*.cpp)
OBJECTS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(SOURCES))

WX_SOURCES   := $(wildcard $(SRC_DIR)/Ui/*.cpp)
WX_OBJECTS   := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(WX_SOURCES))

uiclient: $(OBJECTS) $(WX_OBJECTS)
	@mkdir -p $(EXE_DIR)
	g++ $(LFLAGS) $(SRC_DIR)/ui-client.cpp -o ./build/executables/uiclient $(OBJECTS) $(WX_OBJECTS) $(WX_FLAGS) $(FLAGS)

server: $(OBJECTS)
	@mkdir -p $(EXE_DIR)
	g++ $(LFLAGS) $(SRC_DIR)/fss-server.cpp -o ./build/executables/server $(OBJECTS) $(FLAGS)

clear:
	rm $(OBJECTS)

info:
	@echo $(SOURCES)
	@echo $(OBJECTS)
	@echo $(WX_SOURCES)
	@echo $(WX_OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo building "$@"
	@if [ "$(dir $@)" = "build/obj/Ui/" ]; then g++ $(LFLAGS) -c -o $@ $^ $(WX_FLAGS) $(FLAGS); fi;
	@if [ "$(dir $@)" != "build/obj/Ui/" ]; then g++ $(LFLAGS) -c -o $@ $^ $(FLAGS); fi;
