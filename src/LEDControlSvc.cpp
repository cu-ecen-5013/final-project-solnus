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

using std::cout;
using std::endl;

// Options
static bool _daemon = false;
static uint32_t _ledCount = 0;
static const char* _exe = nullptr;
static bool running = true;

// Forward-declarations
static void usage();
static void parseOpts(int argc, char* const* argv);
static void signal_handler(void);
static void kill_handler(int signum);


// Service entry point
int main(const int argc, char* const* argv)
{
    _exe = argv[0];
    openlog(_exe, LOG_PID|LOG_NDELAY, LOG_USER);

    LOG(LOG_INFO, "Starting service");
    parseOpts(argc, argv);

    // No point starting with no LEDs in use
    if(_ledCount == 0)
    {
        usage();
        LOG(LOG_ERR, "Option: invalid LED count %u", _ledCount);
        closelog();
        exit(1);
    }

    // Daemonize if necessary
    if(_daemon)
    {
        int ret = daemon(0, _veryVerbose);
        if(ret != 0)
        {
            LOG(LOG_ERR, "Failed to daemonize");
            closelog();
            exit(1);
        }

        LOG(LOG_DEBUG, "Daemonized");
    }
    //Pthread device code
    pthread_t tid;
     

    
    LEDControl ctrlObj(_ledCount);
    ctrlObj.setIntensity(0.75);

    // TODO sleep until something happens
    
    // TODO implement signal handling for shutdown
    signal_handler();
    while(running){
        ctrlObj.setAll(LEDControl::LED_R);
        sleep(1);
        ctrlObj.setAll(LEDControl::LED_G);
        sleep(1);
        ctrlObj.setAll(LEDControl::LED_B);
        sleep(1);
        ctrlObj.setAll(LEDControl::LED_W);
        sleep(1);
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
                if(_ledCount == 0)
                {
                    usage();
                    LOG(LOG_ERR, "Option: invalid LED count %u", _ledCount);
                    closelog();
                    exit(1);
                }
                LOG(LOG_DEBUG, "Option: LED count %u", _ledCount);
                break;

            case 'd':
                _daemon = true;
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