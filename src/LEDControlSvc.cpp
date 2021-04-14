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
#include <string.h>

#include "LEDControlSvc_RC.h"

using std::cout;
using std::endl;
#define MAX_ARG_NUM 100
#define MAX_ARG_LEN 100
// Options
//static bool _daemon = false;
static uint32_t _ledCount = 0;
static const char* _exe = nullptr;
static bool running = true;

//can put this to the class
uint32_t _ledColor_in = 0x00002000;
float intensity;
static bool _notdmode = true;
enum stateOps 
{
    nLEDnum      = 1,
    setdaemon    = 2,
    nIntensity   = 3,
    ncolor       = 4,
    pattern      = 5,
    defulteffect = 6,
    cmdprocess   = 7
};
int  state = defulteffect;
char *revstr;
char *argvStore[MAX_ARG_LEN];




// Forward-declarations
static void usage();
static void parseOpts(int argc, char** argv);
static void signal_handler(void);
static void kill_handler(int signum);
static void LEDnumcheck();
static void daemonize();
static void* DeviceRC(void *arg);
static void CountUsrInput(int &wc);
static void freeStr(int narg);
static void resetStr(int &narg,bool isinit);
static void printargvS(int narg);
// Service entry point
int main(const int argc, char** argv)
{   
    
    _exe = argv[0];
    openlog(_exe, LOG_PID|LOG_NDELAY, LOG_USER);
    LOG(LOG_INFO, "Starting service");
    parseOpts(argc, argv);
    LEDControl ctrlObj(_ledCount);
    
    signal_handler();
    int cpidx = 0;
    LEDControl::led_color_e colorPattern [4] = { LEDControl::LED_R, LEDControl::LED_G,
                                  LEDControl::LED_B, LEDControl::LED_W};
    
    int narg = 0;
    pthread_t tid;
    resetStr(narg, true);
    
    assert(pthread_create(&tid,		    //Store ID of the new thread
                        NULL,		    //Default attribute
                        DeviceRC,	    //Start routine
                        (void *)0)==0); //Arg pass to the start routine 
    
    while(running){
        switch (state){
            // daemon the program, allow set once
            case setdaemon:
                daemonize();
                LOG(LOG_DEBUG,"daemon state\n");
                state = cmdprocess;
                break;
            // set number of LED
            case nLEDnum:
                LEDnumcheck();
                ctrlObj.setNumLED(_ledCount);
                LOG(LOG_DEBUG,"nLEDnum state\n");
                state = cmdprocess;
                break;
                
            // Set new intencity
            case nIntensity:
                ctrlObj.setIntensity(intensity);
                state = cmdprocess;
                LOG(LOG_DEBUG,"nIntensity state\n");
                break;

            //Set new color
            case ncolor:
                ctrlObj.setNewColor(_ledColor_in);
                state = cmdprocess;
                LOG(LOG_DEBUG,"ncolor state\n");
                break;
            
            //default effect
            case defulteffect:
                ctrlObj.setAll(colorPattern[cpidx]);
                sleep(1);
                cpidx = cpidx < 3? cpidx+1:0;
                LOG(LOG_DEBUG,"default \n");
                state = cmdprocess; 
                break;

            case cmdprocess:
                if(strcmp(revstr,SPACE_STRING)==0){
                    state = defulteffect;
                }else{
                    CountUsrInput(narg);
                    printargvS(narg);
                    parseOpts(narg, argvStore);
                    resetStr(narg, false);
                }
                break;
            
            default:
                break;
                
        }
    }
    freeStr(narg);
    LOG(LOG_INFO, "Stopping service"); 
    return 0;

}


void parseOpts(int argc, char** argv)
{
    int opt,Intmp;
    optind = 0;
    while((opt = getopt(argc, argv, "n:i:c:dhvV")) != -1)
    {
        switch(opt)
        {
            case 'n':
                _ledCount = strtoul(optarg, nullptr, 10);
                LEDnumcheck();
                state = nLEDnum;
                LOG(LOG_DEBUG, "Option: LED count %u", _ledCount);
                break;

            case 'd':
                //_daemon = true;
                state = setdaemon;
                LOG(LOG_DEBUG, "Option: daemon mode");
                break;
            
            case 'c':
                _ledColor_in = strtoul(optarg, nullptr, 10);
                state = ncolor;
                LOG(LOG_DEBUG, "Option: change color");
                break;
            
            case 'i':
                Intmp = strtoul(optarg, nullptr, 10);
                if ((Intmp >=0) && (Intmp <=100)){
                    intensity =((float)Intmp/(float)100);
                    state = nIntensity;
                    LOG(LOG_DEBUG, "Option: change intensity");
                }else{
                    state = defulteffect;
                }
                break;
            //what is verbose mode
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
    cout << "\t-c#\tChange color" << endl;
    cout << "\t-i#\tChange intensity (required 0-100)" << endl;
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
    if(_notdmode)
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
    // No point starting with no LEDs in use
    if(_ledCount == 0)
    {
        usage();
        LOG(LOG_ERR, "Option: invalid LED count %u", _ledCount);
        closelog();
        exit(1);
    }
}

static void* DeviceRC(void *arg){
    IOTHUB_DEVICE_CLIENT_LL_HANDLE device_ll_handle;
    if(iothub_init(&device_ll_handle)){
        iothub_RC_handler(&device_ll_handle,revstr);
        LOG(LOG_ERR,"Waiting for message to be sent to device \r\n");
        while(1){
            iothub_receive(&device_ll_handle);
        }
    }
    iothub_deinit(&device_ll_handle);
    pthread_exit(NULL);
}

static void freeStr(int narg){
    if(revstr!=NULL){
        free(revstr);
        revstr = NULL;
    }
    
    for(int i=0; i<narg;i++){
        if(argvStore[narg]!=NULL){
            free(argvStore[narg]);  
            argvStore[narg]=NULL;
        }
    }
}
static void resetStr(int &narg, bool isinit){
    if(!isinit){
        freeStr(narg);
    }
    revstr = strdup(SPACE_STRING);   
    for(narg=0;narg<INPUT_OFFSET;narg++ ){
        argvStore[narg] = strdup(DUMMPY_ARG);
        assert(argvStore[narg]!=nullptr);
    }
    
    assert(revstr!=nullptr);
}


static void CountUsrInput(int &wc){
    char delim[] = SPACE_STRING;
	char *token = strtok(revstr, delim);
	while (token != NULL && (wc < MAX_ARG_LEN))
	{
        argvStore[wc] = strdup(token);
		token = strtok(NULL, delim);
        wc++;
	}
}

static void printargvS(int narg){
    LOG(LOG_INFO, "wc is %d, argget is ",narg );
    for (int i = 0; i< narg; i++){
        LOG(LOG_INFO,"%s ",argvStore[i]);
    }
}