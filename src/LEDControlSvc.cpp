/******************************************************************************
 * @file LEDControlSvc.cpp
 * @brief LED Control Service
 * 
 * Defines LED Control Service.
 * 
 * @author Steve Rizor
 * @date 4/3/2021
 * 
 *****************************************************************************/

#include "logging.h"
#include "LEDControl.h"

#include <getopt.h>
#include <syslog.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <assert.h>
#include <unistd.h>

#include "LEDControlSvc_RC.h"
using std::cout;
using std::endl;

// Options
static bool _daemon = false;
static uint32_t _ledCount = 0;
static const char* _exe = nullptr;
static bool running = true;
static IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;

//can put this to the class
static bool _notdmode = true;
enum stateOps 
{
    nLEDnum      = 1,
    setdaemon    = 2,
    nIntensity   = 3,
    ncolor       = 4,
    pattern      = 5,
    defulteffect = 6
};
int  state = defulteffect;
char foo[] = "hello world"; //make complaints revStr is not init.
void *revStr = foo;
void *preStr = revStr;

// Forward-declarations
static void usage();
static void parseOpts(int argc, char* const* argv);
static void signal_handler(void);
static void kill_handler(int signum);
static void LEDnumcheck();
static void daemonize();
static void* DeviceRC(void *arg);


// Service entry point
int main(const int argc, char* const* argv)
{   
    _exe = argv[0];
    openlog(_exe, LOG_PID|LOG_NDELAY, LOG_USER);
    LOG(LOG_INFO, "Starting service");
    parseOpts(argc, argv);
    LEDControl ctrlObj(_ledCount);
    LEDControl::led_t _ledColor_in;
    _ledColor_in.wrgb = 0x00002000;
    signal_handler();
    int cpidx = 0;
    LEDControl::led_color_e colorPattern [4] = { LEDControl::LED_R, LEDControl::LED_G,
                                  LEDControl::LED_B, LEDControl::LED_W};
    if(iothub_init(&device_ll_handle)){
        iothub_RC_handler(&device_ll_handle,revStr);
        pthread_t tid;
        assert(pthread_create(&tid,		    //Store ID of the new thread
                            NULL,		    //Default attribute
                            DeviceRC,	    //Start routine
                            (void *)0)==0); //Arg pass to the start routine
    }
    while(running){
        switch (state){
            // daemon the program, allow set once
            case setdaemon:
                daemonize();
                preStr = revStr;
                state = defulteffect;
                break;

            // set number of LED
            case nLEDnum:
                LEDnumcheck();
                ctrlObj.setNumLED(_ledCount);
                memcpy(preStr,revStr,strlen((char*)revStr));
                state = defulteffect;
                break;
                
            // Set new intencity
            case nIntensity:
                ctrlObj.setIntensity(0.75);
                preStr = revStr;
                memcpy(preStr,revStr,strlen((char*)revStr));
                state = defulteffect;
                break;

            //Set new color
            case ncolor:
                ctrlObj.setNewColor(_ledColor_in);
                preStr = revStr;
                memcpy(preStr,revStr,strlen((char*)revStr));
                state = defulteffect;
                break;
            
            //default effect
            case defulteffect:
                ctrlObj.setAll(colorPattern[cpidx]);
                sleep(1);
                cpidx = cpidx < 3? cpidx+1:0;
                state = 0; //to default
                break;

            default:
                //no new data  
                if(memcmp(revStr,preStr,strlen((char*)preStr))==0){
                    state = defulteffect;
                    printf("no data received: %s\n", (char*)revStr);
                }else{
                    printf("new data received: %s\n", (char*)revStr);
                    //to new state state = ?
                    state = defulteffect;
                }
                break;
                
        }
    }
    LOG(LOG_INFO, "Stopping service"); 
    return 0;
}

void parseOpts(int argc, char* const* argv)
{
    int opt;
    while((opt = getopt(argc, argv, "n:dhvV")) != -1)
    {
        switch(opt)
        {
            case 'n':
                _ledCount = strtoul(optarg, nullptr, 10);
                // No point starting with no LEDs in use
                LEDnumcheck();
                LOG(LOG_DEBUG, "Option: LED count %u", _ledCount);
                break;

            case 'd':
                _daemon = true;
                state = setdaemon;
                LOG(LOG_DEBUG, "Option: daemon mode");
                break;

            case 'v':
                _verbose = true;
                LOG(LOG_DEBUG, "Option: verbose mode");
                break;

            case 'V':
                _veryVerbose = true;
                LOG(LOG_DEBUG, "Option: very verbose mode");
                break;
            default:
                usage();
                closelog();
                exit(1);
        }
    }
}

void usage()
{
    cout << _exe << " [options] -n#" << endl;
    cout << "\t-n#\tNumber of LEDs in the strand (required, >0)" << endl;
    cout << "\t-d\tDaemonize the service" << endl;
    cout << "\t-v\tVerbose logging" << endl;
    cout << "\t-V\tVery verbose logging (stdout)" << endl;
    cout << "\t-h\tUsage" << endl;
    cout << endl;
}

static void kill_handler(int signum){
    running = false;
}
static void signal_handler(){
    struct sigaction sa;
    sa.sa_handler = &kill_handler,
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

static void daemonize(){
    if(_daemon && _notdmode)
    {
        int ret = daemon(0, _veryVerbose);
        if(ret != 0)
        {
            LOG(LOG_ERR, "Failed to daemonize");
            closelog();
            exit(1);
        }
        _notdmode = false;
        LOG(LOG_DEBUG, "Daemonized");
    }
}

static void LEDnumcheck(){
    if(_ledCount == 0)
    {
        usage();
        LOG(LOG_ERR, "Option: invalid LED count %u", _ledCount);
        closelog();
        exit(1);
    }
}

static void* DeviceRC(void *arg){
    while(1){
        iothub_receive(&device_ll_handle);
    }
    iothub_deinit(&device_ll_handle);
    pthread_exit(NULL);
}


