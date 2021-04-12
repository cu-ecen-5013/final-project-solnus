#include "iothub.h"
#include "iothub_device_client_ll.h"
#include "iothub_client_options.h"
#include "iothub_message.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/shared_util_options.h"
#include "iothubtransportmqtt.h"
IOTHUBMESSAGE_DISPOSITION_RESULT receive_msg_callback(IOTHUB_MESSAGE_HANDLE message, void* user_context);
int iothub_init(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle);
void iothub_deinit(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle);
void iothub_RC_handler(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle, void  *revStr);
void iothub_receive(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle);
void iothub_deinit(IOTHUB_DEVICE_CLIENT_LL_HANDLE *device_ll_handle);
void test();