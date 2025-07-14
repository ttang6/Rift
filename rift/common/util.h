#ifndef RIFT_COMMON_UTIL_H
#define RIFT_COMMON_UTIL_H

#include <sys/types.h>
#include <unistd.h>

namespace rift{
    pid_t getPid();
    
    pid_t getThreadId();
}

#endif