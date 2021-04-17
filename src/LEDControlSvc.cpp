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

// Options
static uint32_t _ledCount = 0;
static const char* _exe = nullptr;
static bool running = true;
bool fade = false;
bool off = false;

uint32_t _ledColor_in = 0x00002000;
float intensity = MAX_INTEN_RATE;

int slptime = 1000;
static bool _notdmode = true;
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
int  state = setNorm;
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
static void LEDColorOn(LEDControl::led_t *dPattern, int &dpidx, LEDControl &ctrlObj);
static void ParamReset( LEDControl &ctrlObj);
static void FadeEffect(float &frate, LEDControl &ctrlObj,LEDControl::led_t *dPattern, int &dpidx);
static void selstate();


// Service entry point
int main(const int argc, char** argv)
{   
    
    _exe = argv[0];
    openlog(_exe, LOG_PID|LOG_NDELAY, LOG_USER);
    LOG(LOG_INFO, "Starting service");
    parseOpts(argc, argv);
    LEDControl ctrlObj(_ledCount);
    
    signal_handler();
    int dpidx = 0;                      //display color index
    int scidx = 0;                      //Set color index
    float frate = MAX_INTEN_RATE;        //fade rate
    LEDControl::led_t dPattern [COLOR_PAT_NUM] 
    = { LEDControl::LED_R, LEDControl::LED_G,
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
            case setDaemon:
                daemonize();
                LOG(LOG_DEBUG,"daemon state\n");
                state = setCmdPs;
                break;
            
            // set number of LED
            case setLEDnum:
                LEDnumcheck();
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
                    parseOpts(narg, argvStore);
                    resetStr(narg, false);
                }
                break;
            
            default:
                break;
        }
    }
    ctrlObj.setAllOff();
    freeStr(narg);
    LOG(LOG_INFO, "Stopping service"); 
    return 0;
}


void parseOpts(int argc, char** argv)
{
    static bool FirstIn = true; 
    int opt,Intmp;
    optind = 0;
    while((opt = getopt(argc, argv, "n:i:c:s:ofdhvVrx")) != -1)
    {
        switch(opt)
        {
            case 'n':
                _ledCount = strtoul(optarg, nullptr, 10);
                LEDnumcheck();
                state = setLEDnum;
                LOG(LOG_DEBUG, "Option: LED count %u", _ledCount);
                break;

            case 'd':
                state = setDaemon;
                LOG(LOG_DEBUG, "Option: daemon mode");
                break;
            
            case 'r':
                fade = false;    
                off = false;
                state = setNorm;
                LOG(LOG_DEBUG, "Option: set to regular flash effect");
                break;

            case 'c':
                _ledColor_in = strtoul(optarg, nullptr, 10);
                state = setColor;
                LOG(LOG_DEBUG, "Option: change LED color");
                break;
            
            case 'i':
                Intmp = strtoul(optarg, nullptr, 10);
                if ((Intmp >=0) && (Intmp <=100)){
                    intensity =((float)Intmp/(float)PERCENTAGE);
                    state = setInten;
                    LOG(LOG_DEBUG, "Option: change intensity");
                }
                break;
            case 's':
                slptime = strtoul(optarg, nullptr, 10);
                state = setNorm;
                LOG(LOG_DEBUG, "Option: change LED flash frequency");
                break;

            case 'f':
                off = false;
                fade = true;
                state = setFade;
                LOG(LOG_DEBUG, "Option: set to fade effect"); 
                break;

            case 'o':
                fade = false;    
                off = true;
                state = LEDoff;
                LOG(LOG_DEBUG, "Option: turn off all LEDs"); 
                break;

            case 'x':
                running = false;
                LOG(LOG_DEBUG, "Option: shunt down the process"); 
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
                //False UI input will be ignore
                if (FirstIn){
                    closelog();
                    exit(1);
                }
        }
    }
    FirstIn = false;
}

void usage()
{
    cout << _exe << " [options] -n#" << endl;
    cout << "\t-n#\tNumber of LEDs to turn on in the strand (required, >0)" << endl;
    cout << "\t-i#\tChange intensity (required 0-100)" << endl;
    cout << "\t-c#\tChange color" << endl;
    cout << "\t-s#\tChange sleep time" << endl;
    cout << "\t-f\tFade effect" << endl;
    cout << "\t-r\tRegular effect" << endl;
    cout << "\t-o\tTurn off LED" << endl;
    cout << "\t-x\tshut down the entire process" << endl;
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
//Thread function for receiving UI input 
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

static void LEDColorOn(LEDControl::led_t *dPattern, int &dpidx, LEDControl &ctrlObj){
    ctrlObj.setNewColor(dPattern[dpidx]);
    usleep(slptime*US_TO_MS);
    dpidx = dpidx < COLOR_PAT_NUM-1? dpidx+1:0;
}

static void ParamReset( LEDControl &ctrlObj){
    intensity = (int)MAX_INTEN_RATE;
    slptime = DEFAULT_SLEEP;
    ctrlObj.setIntensity(intensity);
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