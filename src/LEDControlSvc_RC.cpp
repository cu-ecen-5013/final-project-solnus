// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "LEDControlSvc_RC.h"

#define SAMPLE_MQTT


/* Paste in the your iothub connection string  */
static const char* connectionString = "HostName=ecen5713-iot-hub.azure-devices.net;DeviceId=" AZURE_DEVICE_NAME ";SharedAccessKey=" AZURE_DEVICE_KEY;
IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
    //user_context = (char*) user_context;
    IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(message);
    if (content_type == IOTHUBMESSAGE_BYTEARRAY)
    {
        const unsigned char* buff_msg;
        size_t buff_len;

        if (IoTHubMessage_GetByteArray(message, &buff_msg, &buff_len) != IOTHUB_MESSAGE_OK)
        {
            LOG(LOG_ERR,"Failure retrieving byte array message\r\n");
        }
        else
        {
            user_context = realloc(user_context,(buff_len*sizeof(char))+1);
            memcpy((user_context),buff_msg,buff_len);
	        ((char*)user_context)[buff_len]  = '\0';
            LOG(LOG_DEBUG,"Received Binary message\r\n Data1: %.*s\r\n", (int)buff_len, (char*)user_context);
        }
    }
    else
    {
        const char* string_msg = IoTHubMessage_GetString(message);
        if (string_msg == NULL)
        {
            LOG(LOG_ERR,"Failure retrieving byte array message\r\n");
        }
        else
        {
            user_context = realloc(user_context,(strlen(string_msg)*sizeof(char))+1);
            memcpy(user_context,string_msg,strlen(string_msg));
	        ((char*)user_context)[strlen(string_msg)]  = '\0';
            LOG(LOG_DEBUG, "Received String Message\r\nData: %s\r\n", (char*)user_context);
        }
    }
    return IOTHUBMESSAGE_ACCEPTED;
}

int iothub_init(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol;
    protocol = MQTT_Protocol;
    // Used to initialize IoTHub SDK subsystem
    (void)IoTHub_Init();
    LOG(LOG_INFO,"Creating IoTHub Device handle\r\n");
    // Create the iothub handle here
    *device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
    if (*device_ll_handle == NULL)
    {
        LOG(LOG_ERR,"Failure creating IotHub device. Hint: Check your connection string.\r\n");
        return 0;
    }
    LOG(LOG_INFO, "IoT hub init successfully\r\n");
    return 1;
}

void iothub_RC_handler(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle, void  *revStr){
    bool urlDecodeOn = true;
    (void)IoTHubDeviceClient_LL_SetOption(*device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlDecodeOn);
    if (IoTHubDeviceClient_LL_SetMessageCallback(*device_ll_handle, receive_msg_callback, revStr) != IOTHUB_CLIENT_OK)
    {
        LOG(LOG_ERR, "ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
        return ;
    }
}
void iothub_receive(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle){
    IoTHubDeviceClient_LL_DoWork(*device_ll_handle);
    ThreadAPI_Sleep(2000);
}

void iothub_deinit(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle){
    LOG(LOG_INFO,"Exiting...");
    IoTHubDeviceClient_LL_Destroy(*device_ll_handle);
    IoTHub_Deinit();
}

void test(){
    char foo[] = "hello world"; //make complaints revStr is not init.
    void  *revStr = foo;
    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;
    if(iothub_init(&device_ll_handle)){
        iothub_RC_handler(&device_ll_handle,revStr);
        LOG(LOG_INFO,"Waiting for message to be sent to device \r\n");
        while(1){
            iothub_receive(&device_ll_handle);
        }
    }
    iothub_deinit(&device_ll_handle);
}
