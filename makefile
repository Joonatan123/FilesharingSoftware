LFLAGS = -I./include/
FLAGS = -l:libboost_system.a -l:libboost_filesystem.a -l:libboost_thread.a -l:libboost_chrono.a -l:libboost_serialization.a -lpthread
WX_FLAGS = `wx-config --cxxflags --libs std`

SRC_DIR := ./src
OBJ_DIR := ./build/obj
EXE_DIR := ./build/executables

SOURCES   := $(wildcard $(SRC_DIR)/*/*.cpp)
OBJECTS   := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o,$(SOURCES))

uiclient: $(OBJECTS)
	@mkdir -p $(EXE_DIR)
	g++ $(LFLAGS) $(SRC_DIR)/ui-client.cpp -o ./build/executables/uiclient $(OBJECTS) $(WX_FLAGS) $(FLAGS)

server: $(OBJECTS)
	@mkdir -p $(EXE_DIR)
	g++ $(LFLAGS) $(SRC_DIR)/fss-server.cpp -o ./build/executables/server $(OBJECTS) $(FLAGS)

clear:
	rm $(OBJECTS)

info:
	@echo $(SOURCES)
	@echo $(OBJECTS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo building "$@"
	@if [ "$(dir $@)" = "build/obj/Ui/" ]; then g++ $(LFLAGS) -c -o $@ $^ $(WX_FLAGS) $(FLAGS); fi;
	@if [ "$(dir $@)" != "build/obj/Ui/" ]; then g++ $(LFLAGS) -c -o $@ $^ $(FLAGS); fi;
