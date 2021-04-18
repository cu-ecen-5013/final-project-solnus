// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.


#include <stdio.h>
#include <stdlib.h>

#include "logging.h"
#include "LEDControlSvc_RC.h"

#define SAMPLE_MQTT

LEDControlSvc_RC::LEDControlSvc_RC(const char* connStr, LEDControl& ctrlObj)
    :_connStr(connStr), _ctrlObj(ctrlObj)
{
}

void LEDControlSvc_RC::_run()
{
    _iothub_init();

    while(_running)
    {
        IoTHubDeviceClient_LL_DoWork(_device_ll_handle);
        ThreadAPI_Sleep(_cyclePeriodMs);
    }

    _iothub_deinit();
}

void LEDControlSvc_RC::_iothub_init()
{
    // Initialize IoTHub SDK subsystem
    if(IoTHub_Init() != 0)
    {
        LOG(LOG_ERR, "Failed to initialize IoTHub");
        exit(1);
    }

    // Create the iothub handle here
    LOG(LOG_INFO,"Creating IoTHub Device handle\r\n");
    _device_ll_handle = IoTHubDeviceClient_LL_CreateFromConnectionString(_connStr, _protocol);
    if (_device_ll_handle == NULL)
    {
        LOG(LOG_ERR,"Failure creating IotHub device. Hint: Check your connection string.\r\n");
        exit(1);
    }

    bool urlDecodeOn = true;
    if (IoTHubDeviceClient_LL_SetOption(_device_ll_handle, OPTION_AUTO_URL_ENCODE_DECODE, &urlDecodeOn) != IOTHUB_CLIENT_OK)
    {
        LOG(LOG_ERR, "IoTHub SetOption failed")
        exit(1);
    }

    if (IoTHubDeviceClient_LL_SetMessageCallback(_device_ll_handle, _receive_msg_callback, (void*)this) != IOTHUB_CLIENT_OK)
    {
        LOG(LOG_ERR, "IoTHub SetMessageCallback failed");
        exit(1);
    }

    LOG(LOG_INFO, "IoT hub init successfully");
}

void LEDControlSvc_RC::_iothub_deinit(){
    LOG(LOG_INFO,"Exiting...");
    IoTHubDeviceClient_LL_Destroy(_device_ll_handle);
    IoTHub_Deinit();
}

void LEDControlSvc_RC::_handle_message(const unsigned char* buffer, size_t size)
{
    char string_msg[80];
    if(size >= sizeof(string_msg))
    {
        LOG(LOG_ERR, "Received a message that's too long: %zu", size);
        return;
    }

    strncpy(string_msg, (const char*)buffer, size);
    string_msg[size] = '\0';

    LOG(LOG_DEBUG, "Received String Message: %s", string_msg);

    if(strstr(string_msg, "intensity2"))
    {
        _ctrlObj.setIntensity(0.2);
    }
    else if(strstr(string_msg, "intensity5"))
    {
        _ctrlObj.setIntensity(0.5);
    }
    else if(strstr(string_msg, "colorg"))
    {
        _ctrlObj.setColor(LEDControl::LED_G);
    }
    else if(strstr(string_msg, "colorr"))
    {
        _ctrlObj.setColor(LEDControl::LED_R);
    }
}

/*
#define MAX_ARG_NUM     100
#define MAX_ARG_LEN     100
#define COLOR_PAT_NUM   4
#define MAX_INTEN_RATE  1.0
#define US_TO_MS        1000
#define NS_TO_MS        1000000
#define DEFAULT_SLEEP   1000
#define PERCENTAGE      100
#define FADE_STEP       0.01//1/(float)FADE_RES
#define SLEEP_RES       10
#define FADE_RES        100 
static void LEDColorOn(LEDControl::led_t *dPattern, int &dpidx, LEDControl &ctrlObj);
static void FadeEffect(float &frate, LEDControl &ctrlObj,LEDControl::led_t *dPattern, int &dpidx);
static void selstate();

bool _fade = false;
bool _off = false;

uint32_t _ledColor_in = 0x00002000;
float _intensity = MAX_INTEN_RATE;


static void LEDnumcheck(){
    // No point starting with no LEDs in use
    if(_ledCount == 0)
    {
        usage();
        LOG(LOG_ERR, "Option: invalid LED count %u", _ledCount);
        closelog();
        exit(1);
    }
}

static void LEDColorOn(LEDControl::led_t *dPattern, int &dpidx, LEDControl &ctrlObj){
    ctrlObj.setNewColor(dPattern[dpidx]);
    usleep(slptime*US_TO_MS);
    dpidx = dpidx < COLOR_PAT_NUM-1? dpidx+1:0;
}

static void FadeEffect(float &frate, LEDControl &ctrlObj,LEDControl::led_t *dPattern, int &dpidx){
    bool up = false;
    struct timespec ts;
    for(int i =0; i<FADE_RES; i++){
        assert(clock_gettime(CLOCK_MONOTONIC, &ts)==0);
        //intented to use slptime to SLEEP_RES, but flash frequency is not changin?
        ts.tv_nsec=NS_TO_MS*SLEEP_RES; 
        if(up){
            frate+=FADE_STEP;
            if(frate>=MAX_INTEN_RATE)
                up = !up;
        } else{
            frate-=FADE_STEP;
            if(frate<=0)
                up = !up;
        }
        ctrlObj.setIntensity(frate);
        ctrlObj.setNewColor(dPattern[dpidx]);
        assert(clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL)==0);
    }
    ParamReset(ctrlObj);
    dpidx = dpidx < COLOR_PAT_NUM-1? dpidx+1:0; 
}

static void selstate(){
    if(fade)
        state = setFade;
    else if(off)
        state = LEDoff;
    else
        state = setNorm;
}

enum stateOps 
{
    setLEDnum = 1,
    setDaemon,  
    setInten,      
    setColor,    
    setPttrn,    
    setFade,
    setNorm,
    LEDoff,
    shutdown,
    setCmdPs  
};
int  _state = setNorm;
char* _revstr;
char* _argvStore[MAX_ARG_LEN];
*/

/*

        switch (state){
            // set number of LEDs
            case setLEDnum:
                ctrlObj.setAllOff();
                ctrlObj.setNumLED(_ledCount);
                ctrlObj.setIntensity(intensity);
                state = setCmdPs;
                LOG(LOG_DEBUG,"setLEDnum state\n");
                break;

            // Set new intensity
            case setInten:
                ctrlObj.setIntensity(intensity);
                state = setCmdPs;
                LOG(LOG_DEBUG,"setInten state\n");
                break;

            //Set new color to array
            //recomand convert from hex to decimal and sent decimal string from UI
            case setColor:
                dPattern[scidx].wrgb = _ledColor_in;
                scidx = scidx < (COLOR_PAT_NUM -1) ? scidx+1:0;
                state = setCmdPs;
                LOG(LOG_DEBUG,"setColor state\n");
                break;
            
            // Set all LED off
            case LEDoff:
                ctrlObj.setAllOff();
                state = setCmdPs;
                LOG(LOG_DEBUG,"LEDoff state\n");
                break;
            
            //set fade paramters
            case setFade:
                FadeEffect(frate, ctrlObj,dPattern,dpidx);
                state = setCmdPs;
                LOG(LOG_DEBUG,"LED fade effect\n");
                break; 

            //set default LED 
            case setNorm:
                LEDColorOn(dPattern, dpidx, ctrlObj);
                state = setCmdPs; 
                LOG(LOG_DEBUG,"setNorm \n");
                break;
            // Command process
            case setCmdPs:
                if(strcmp(revstr,SPACE_STRING)==0){
                   selstate();
                }else{
                    ParamReset(ctrlObj);
                    CountUsrInput(narg);
                    printargvS(narg);
                    resetStr(narg, false);
                }
                break;
            
            default:
                break;
        }
*/