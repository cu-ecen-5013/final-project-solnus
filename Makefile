SRC_DIR := src
SOURCES := $(wildcard $(SRC_DIR)/*.cpp)
BUILD_DIR := build
WSLIB := rpi_ws281x/libws2811.a
OBJS := $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o) $(WSLIB)
BIN_FILE := $(BUILD_DIR)/LEDControlSvc

ifeq ($(CXX),)
	CXX = g++
endif

ifeq ($(SCONS),)
	SCONS = scons
endif

ifeq ($(AZURE_DEVICE_KEY),)
	AZURE_DEVICE_KEY = "Gply81vOUaYpgBujYD0xhIk13xzExrKSf0EDvTW0LDA="
endif

ifeq ($(AZURE_DEVICE_NAME),)
	AZURE_DEVICE_NAME = "LEDControlSvc2"
endif

ifeq ($(EXTRA_CXXFLAGS),)
	EXTRA_CXXFLAGS = -Wall -Werror -std=c++17 -I. -I$(STAGING_DIR)/usr/include/azureiot -DAZURE_DEVICE_KEY=\"$(AZURE_DEVICE_KEY)\" -DAZURE_DEVICE_NAME=\"$(AZURE_DEVICE_NAME)\"
endif


# If building from buildroot, linking happens differently
ifeq ($(BR2_CONFIG),)
	AZURELIBS := -liothub_client_mqtt_transport -liothub_client_amqp_transport -liothub_client_http_transport -liothub_client -lumqtt -lprov_auth_client -lhsm_security_client -laziotsharedutil -lparson -luhttp -luuid -lpthread -lcurl -lssl -lcrypto
else
	AZURELIBS := -lumqtt -luamqp -laziotsharedutil -liothub_client -liothub_client_mqtt_ws_transport -liothub_client_mqtt_transport -liothub_service_client
endif

all: $(BUILD_DIR) $(BIN_FILE)

$(BIN_FILE): $(OBJS)
	$(CXX) $(CXXFLAGS) $(EXTRA_CXXFLAGS) -o $@ $^ $(INCLUDES) -L$(STAGING_DIR)/usr/lib $(LDFLAGS) $(AZURELIBS)

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
