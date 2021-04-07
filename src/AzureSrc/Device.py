import threading
import time
import json
import subprocess
import sys
from azure.iot.device import IoTHubDeviceClient
RECEIVED_MESSAGES = 0
CONNECTION_STRING = "HostName=ecen5713-iot-hub.azure-devices.net;DeviceId=ecen5713-iot-edge;SharedAccessKey=sVDMKlFJsDETVlACDeH4iav64ks+5ejIw/kTvSag354="


def message_listener(client):
    global RECEIVED_MESSAGES
    while True:
        message = client.receive_message()
        RECEIVED_MESSAGES += 1
        print("\nMessage received:")
        bytestr = list(vars(message).values())
        data = bytestr[0].decode("UTF-8")
        print("    {0}".format(data))
        p1 = subprocess.call(data,shell=True)
        
        
        #print data and both system and application (custom) properties
        #for property in vars(message).items():
        #    print ("    cmd{0}".format(property))
        print( "    Total calls received: {}".format(RECEIVED_MESSAGES))

def iothub_client_run():
    try:
        client = IoTHubDeviceClient.create_from_connection_string(CONNECTION_STRING)

        message_listener_thread = threading.Thread(target=message_listener, args=(client,))
        message_listener_thread.daemon = True
        message_listener_thread.start()

        while True:
            time.sleep(1000)

    except KeyboardInterrupt:
        print ( "IoT Hub C2D Messaging device stopped" )

if __name__ == '__main__':
    print ( "Starting the Python IoT Hub C2D Messaging device ..." )
    print ( "Waiting for C2D messages" )

    iothub_client_run()
