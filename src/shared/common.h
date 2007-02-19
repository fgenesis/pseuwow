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

#define SDL_THREADS_DISABLED true

//#include "SDL/SDL.h" // disabled for now until needed
#include "zthread/FastMutex.h"
#include "zthread/LockedQueue.h"
#include "zthread/Runnable.h"
#include "zthread/Thread.h"

#include "SysDefs.h"
#include "DebugStuff.h"
#include "HelperDefs.h"
#include "tools.h"

#endif

