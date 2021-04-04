/******************************************************************************
 * @file logging.h
 * @brief Logging facility
 * 
 * Logging facility for LEDControlSvc. Syslog must be initialized prior to use.
 * 
 * @author Steve Rizor
 * @date 4/3/2021
 * 
 *****************************************************************************/
#pragma once

#include <syslog.h>
#include <stdio.h>

inline bool _verbose;
inline bool _veryVerbose;

// Macros
#define LOG(level, message, ...) {if((level <= LOG_INFO) || _verbose) syslog(level, message, ##__VA_ARGS__);\
                                  if(_veryVerbose) printf(message "\n", ##__VA_ARGS__);}
