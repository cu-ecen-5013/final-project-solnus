SRC_DIR := src
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
BUILD_DIR := build
WSLIB := rpi_ws281x/libws2811.a
AZURELIBS := -liothub_client_mqtt_transport -liothub_client_amqp_transport -liothub_client_http_transport -liothub_client -lumqtt -lprov_auth_client -lhsm_security_client -luhttp -laziotsharedutil -lparson -luuid -lpthread -lcurl -lssl -lcrypto
OBJS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o) $(WSLIB)
BIN_FILE := $(BUILD_DIR)/LEDControlSvc

ifeq ($(CXX),)
	CXX = g++
endif

ifeq ($(SCONS),)
	SCONS = scons
endif

ifeq ($(EXTRA_CXXFLAGS),)
	EXTRA_CXXFLAGS = -Wall -Werror -std=c++17 -I. -I/usr/include/azureiot
endif

all: $(BUILD_DIR) $(BIN_FILE)

$(BIN_FILE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o $@ $^ $(INCLUDES) $(LDFLAGS) -L$(STAGING_DIR)/usr/lib $(AZURELIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -c -o $@ -MMD -MT '$@' -MF $(@:%.o=%.d) $<

$(BUILD_DIR):
	mkdir -p $@

$(WSLIB):
	cd rpi_ws281x && $(SCONS) V=1 TOOLCHAIN=$(TOOLCHAIN)

-include $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.d)

.PHONY: clean all
clean:
	rm -rdf $(BUILD_DIR)
	cd rpi_ws281x && scons -c
