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
#include "LEDControlObj.h"

#include <getopt.h>
#include <syslog.h>
#include <iostream>

using std::cout;
using std::endl;

// Macros
#define LOG(level, message, ...) {if((level <= LOG_INFO) || _verbose) syslog(level, message, ##__VA_ARGS__);\
                                  if(_veryVerbose) printf(message "\n", ##__VA_ARGS__);}


// Options
static bool _daemon = false;
static uint32_t _ledCount = 0;
static bool _verbose = false;
static bool _veryVerbose = false;
static const char* _exe = nullptr;

// Forward-declarations
static void usage();
static void parseOpts(int argc, char* const* argv);

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
