#include "LEDControl.h"

#include <pthread.h>
#include <iothub.h>
#include <iothub_device_client_ll.h>
#include <iothub_client_options.h>
#include <iothub_message.h>
#include <azure_c_shared_utility/threadapi.h>
#include <azure_c_shared_utility/shared_util_options.h>
#include <iothubtransportmqtt.h>

#define EMPTY_STRING ""
#define SPACE_STRING " "
#define DUMMPY_ARG   "dummpy"
#define INPUT_OFFSET 1

class LEDControlSvc_RC
{
public:
    LEDControlSvc_RC(const char* connStr, LEDControl& ctrlObj);

    void start()
    {
        if(_running)
        {
            return;
        }

        _running = true;
        if(pthread_create(&_tid, NULL, _start, (void*)this) != 0)
        {
            LOG(LOG_ERR, "Failed to start RC thread");
            exit(1);
        }
    }

    void stop()
    {
        _running = false;
        pthread_join(_tid, NULL);
    }

private:
    // Constants
    const uint32_t _cyclePeriodMs = 100;
    const char* _connStr;
    IOTHUB_CLIENT_TRANSPORT_PROVIDER _protocol = MQTT_Protocol;

    // Handles/state variables
    pthread_t _tid;
    LEDControl& _ctrlObj;
    IOTHUB_DEVICE_CLIENT_LL_HANDLE _device_ll_handle;
    bool _running = false;
    
    // Member functions
    void _run();
    void _handle_message(const unsigned char* buffer, size_t size);

    void _iothub_init();
    void _iothub_deinit();

    // Static callback functions
    static IOTHUBMESSAGE_DISPOSITION_RESULT _receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
    {
        LEDControlSvc_RC* obj = (decltype(obj))user_context;

        IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(message);
        if (content_type != IOTHUBMESSAGE_BYTEARRAY)
        {
            LOG(LOG_ERR, "Got string message, not handling it");
            return IOTHUBMESSAGE_REJECTED;
        }

        size_t size = 0;
        const unsigned char* buffer = nullptr;
        if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
        {
            LOG(LOG_ERR,"Failure retrieving byte array message");
            return IOTHUBMESSAGE_REJECTED;
        }

        obj->_handle_message(buffer, size);
        return IOTHUBMESSAGE_ACCEPTED;
    }

    static void* _start(void* arg)
    {
        LEDControlSvc_RC* obj = (decltype(obj))arg;
        obj->_run();
        return nullptr;
    }
};
