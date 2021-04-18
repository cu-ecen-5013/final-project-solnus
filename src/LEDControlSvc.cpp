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
#include <libconfig.h>

#include "LEDControlSvc_RC.h"

using std::cout;
using std::endl;
#define CFGFILE "/etc/LEDControlSvc.conf"

// Options
static uint32_t _ledCount = 0;
static const char* _exe = nullptr;
static bool _running = true;
static char _connStr[256];

// Forward-declarations
static void usage();
static void parseOpts(int argc, char** argv);
static void signal_handler(int);

// Service entry point
int main(const int argc, char** argv)
{   
    // Set up logging
    _exe = argv[0];
    openlog(_exe, LOG_PID|LOG_NDELAY, LOG_USER);

    // Set up signal handlers
    struct sigaction sa;
    sa.sa_handler = &signal_handler,
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    LOG(LOG_INFO, "Starting service");
    parseOpts(argc, argv);

    LEDControl ctrlObj(_ledCount);
    ctrlObj.setColor(LEDControl::LED_G);
    ctrlObj.setIntensity(0.1);

    LEDControlSvc_RC remoteCtrlObj(_connStr, ctrlObj);
    remoteCtrlObj.start();

    while(_running){
        pause();
    }

    LOG(LOG_INFO, "Stopping service"); 
    remoteCtrlObj.stop();
    ctrlObj.setIntensity(0);
    LOG(LOG_INFO, "Stopped service"); 
    return 0;
}


void parseOpts(int argc, char** argv)
{
    config_t cfg;
    config_init(&cfg);
    if(!config_read_file(&cfg, CFGFILE))
    {
        LOG(LOG_ERR, "Failed to read config file (%s:%d - %s)\n", config_error_file(&cfg), config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        closelog();
        exit(1);
    }

    const char* devname;
    const char* key;
    if(!config_lookup_string(&cfg, "devname", &devname) || !config_lookup_string(&cfg, "key", &key))
    {
        LOG(LOG_ERR, "Failed to get devname/key from config file");
        config_destroy(&cfg);
        closelog();
        exit(1);
    }

    strcpy(_connStr, "HostName=ecen5713-iot-hub.azure-devices.net;DeviceId=");
    strncat(_connStr, devname, sizeof(_connStr) - strlen(_connStr));
    strncat(_connStr, ";SharedAccessKey=", sizeof(_connStr) - strlen(_connStr));
    strncat(_connStr, key, sizeof(_connStr) - strlen(_connStr));

    int opt;
    optind = 0;
    while((opt = getopt(argc, argv, "n:dhvV")) != -1)
    {
        switch(opt)
        {
            case 'n':
                _ledCount = strtoul(optarg, nullptr, 10);
                LOG(LOG_DEBUG, "Option: LED count %u", _ledCount);
                break;

            case 'd':
                if(daemon(0, _veryVerbose) != 0)
                {
                    LOG(LOG_ERR, "Failed to daemonize");
                    closelog();
                    exit(1);
                }
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

static void signal_handler(int sig){
    _running = false;
}
