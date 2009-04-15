#ifndef _COMMON_H
#define _COMMON_H


#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>
#include <sstream>
#include <list>
#include <deque>
#include <vector>

#ifndef _COMMON_SKIP_THREADS
# include "zthread/Condition.h"
# include "zthread/FastMutex.h"
# include "zthread/FastRecursiveMutex.h"
# include "zthread/LockedQueue.h"
# include "zthread/Runnable.h"
# include "zthread/Thread.h"
#endif

#define STRINGIZE(a) #a

#include "SysDefs.h"
#include "DebugStuff.h"
#include "Widen.h"
#include "tools.h"
#include "log.h"

#include "ByteBuffer.h"

#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif


#endif

