#ifndef DEFS_H_INCLUDED
#define DEFS_H_INCLUDED

#ifndef ASSERT
#   ifdef __DEBUG
#       include <assert.h>
#       define ASSERT(exp)                      assert(exp)
#   else
#       define ASSERT(exp)                      ((void)0)
#   endif
#endif

#ifdef WIN32
#   include <windows.h>
#   define SLEEP(millisec)                      Sleep(millisec)
#else
#   include <unistd.h>
#   define SLEEP(millisec)                      usleep(millisec * 1000)
#endif

#include <stdio.h>

#include <exception>

#include <string>
#include <vector>
#include <list>
#include <map>

#include <utility>
#include <algorithm>

#include <Defs.h>

#include <OZWException.h>

#include <platform/Log.h>

#include <Options.h>
#include <Utils.h>

#undef OZW_FATAL_ERROR
#undef OZW_ERROR

#ifndef OPENZWAVE_DISABLE_EXCEPTIONS
#   define OZW_FATAL_ERROR(exitCode, msg)   	Log::Write(OpenZWave::LogLevel_Error,"Exception: %s:%d - %d - %s", std::string(__MYFUNCTION__).substr(std::string(__MYFUNCTION__).find_last_of("/\\") + 1).c_str(), __LINE__, exitCode, msg); \
                                                if (OpenZWaveMe::OZWException::IsZWayError(exitCode)) \
                                                    throw OpenZWaveMe::OZWException(); \
                                                else throw OpenZWave::OZWException(__MYFUNCTION__, __LINE__, exitCode, msg);
#   define OZW_ERROR(exitCode, msg) 			Log::Write(OpenZWave::LogLevel_Warning,"Exception: %s:%d - %d - %s", std::string(__MYFUNCTION__).substr(std::string(__MYFUNCTION__).find_last_of("/\\") + 1).c_str(), __LINE__, exitCode, msg); \
                                                if (OpenZWaveMe::OZWException::IsZWayError(exitCode)) \
                                                    throw OpenZWaveMe::OZWException(); \
                                                else throw OpenZWave::OZWException(__MYFUNCTION__, __LINE__, exitCode, msg);
#else
#   define OZW_FATAL_ERROR(exitCode, msg)   	Log::Write(OpenZWave::LogLevel_Error,"Exception: %s:%d - %d - %s", std::string(__MYFUNCTION__).substr(std::string(__MYFUNCTION__).find_last_of("/\\") + 1).c_str(), __LINE__, exitCode, msg); \
                                                std::cerr << "Error: "<< std::string(__MYFUNCTION__).substr(std::string(__MYFUNCTION__).find_last_of("/\\") + 1) << ":" << __LINE__ << " - " << msg << std::endl; exit(exitCode);
#   define OZW_ERROR(exitCode, msg)			    Log::Write(OpenZWave::LogLevel_Warning,"Exception: %s:%d - %d - %s", std::string(__MYFUNCTION__).substr(std::string(__MYFUNCTION__).find_last_of("/\\") + 1).c_str(), __LINE__, exitCode, msg);
#endif

typedef uint32 __socklen_t;

using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::list;
using std::map;
using std::pair;
using std::any_of;
using std::for_each;

using OpenZWave::Log;
using OpenZWave::Options;

#endif // DEFS_H_INCLUDED
