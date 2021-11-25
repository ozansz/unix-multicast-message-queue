#ifndef __COMMON_H
#define __COMMON_H

#include <mutex>
#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "constants.h"
#include "errors.h"
#include "error_trace.h"
#include "shared_mem.h"
#include "rwlock.h"

extern std::vector<TraceNode> __error_trace_glob;

#endif
