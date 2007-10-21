#ifndef _COMMON_H
#define _COMMON_H


#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <list>
#include <deque>
#include <vector>

#ifndef _COMMON_SKIP_THREADS
# include "zthread/FastMutex.h"
# include "zthread/LockedQueue.h"
# include "zthread/Runnable.h"
# include "zthread/Thread.h"
#endif

#include "SysDefs.h"
#include "DebugStuff.h"
#include "tools.h"
#include "log.h"

#include "ByteBuffer.h"


#endif

