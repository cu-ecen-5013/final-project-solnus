SRC_DIR := src
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
BUILD_DIR := build
OBJS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
BIN_FILE := $(BUILD_DIR)/LEDControlSvc

ifeq ($(CXX),)
	CXX = g++
endif

ifeq ($(CXXFLAGS),)
	CXXFLAGS = -Wall -Werror
endif

all: $(BUILD_DIR) $(BIN_FILE)

$(BIN_FILE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o $@ $^ $(INCLUDES) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c -o $@ -MMD -MT '$@' -MF $(@:%.o=%.d) $<

$(BUILD_DIR):
	mkdir -p $@

-include $(OBJS:%.o=%.d)

.PHONY: clean all
clean:
	rm -rdf $(BUILD_DIR)
