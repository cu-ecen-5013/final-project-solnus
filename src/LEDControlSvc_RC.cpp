// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// CAVEAT: This sample is to demonstrate azure IoT client concepts only and is not a guide design principles or style
// Checking of return codes and error values shall be omitted for brevity.  Please practice sound engineering practices
// when writing production code.

#include <stdio.h>
#include <stdlib.h>

#include "/home/jiabinlin/Desktop/ECEN5713/application/src/azure-iot-sdk-c/iothub_client/inc/iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"

#define SAMPLE_MQTT


/* Paste in the your iothub connection string  */
static const char* connectionString = "HostName=ecen5713-iot-hub.azure-devices.net;DeviceId=ecen5713-iot-edge;SharedAccessKey=sVDMKlFJsDETVlACDeH4iav64ks+5ejIw/kTvSag354=";
static IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context)
{
//    (void)user_context;
    IOTHUBMESSAGE_CONTENT_TYPE content_type = IoTHubMessage_GetContentType(message);
    if (content_type == IOTHUBMESSAGE_BYTEARRAY)
    {
        const unsigned char* buff_msg;
        size_t buff_len;

        if (IoTHubMessage_GetByteArray(message, &buff_msg, &buff_len) != IOTHUB_MESSAGE_OK)
        {
            (void)printf("Failure retrieving byte array message\r\n");
        }
        else
        {
            memcpy(user_context,buff_msg,buff_len);
            user_context[buff_len] = '\0';
            (void)printf("Received Binary message\r\n Data1: %.*s\r\n", buff_len, (char*)user_context);
        }
    }
    else
    {
        const char* string_msg = IoTHubMessage_GetString(message);
        if (string_msg == NULL)
        {
            (void)printf("Failure retrieving byte array message\r\n");
        }
        else
        {
            memcpy(user_context,string_msg,sizeof(string_msg));
            (void)printf("Received String Message\r\nData: %s\r\n", (char*)user_context);
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
    (void)printf("Creating IoTHub Device handle\r\n");
    // Create the iothub handle here
    *device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(connectionString, protocol);
    if (*device_ll_handle == NULL)
    {
        (void)printf("Failure creating IotHub device. Hint: Check your connection string.\r\n");
        return 0;
    }
    printf("IoT hub init successfully\r\n");
    return 1;
}

void iothub_receive(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle){
    bool urlDecodeOn = true;
    void *revStr;
    (void)IoTHubDeviceClient_LL_SetOption(*device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlDecodeOn);
    if (IoTHubDeviceClient_LL_SetMessageCallback(*device_ll_handle, receive_msg_callback, &revStr) != IOTHUB_CLIENT_OK)
    {
        (void)printf("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!\r\n");
        return ;
    }
    (void)printf("Waiting for message to be sent to device \r\n");
    while(1){
        IoTHubDeviceClient_LL_DoWork(*device_ll_handle);
        ThreadAPI_Sleep(10);
    }
}

void iothub_deinit(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle){
    printf("Exiting...");
    IoTHubDeviceClient_LL_Destroy(*device_ll_handle);
    IoTHub_Deinit();
}

int main2(){
    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

    if(iothub_init(&device_ll_handle)){
        iothub_receive(&device_ll_handle);
    }
    iothub_deinit(&device_ll_handle);
    return 0;
}