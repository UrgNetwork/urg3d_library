#ifndef URG3DTICKS_H
#define URG3DTICKS_H

#include "Urg3dDetectOS.h"
#include <time.h>
#include <stdint.h>

#ifdef URG3D_MAC_OS
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace Urg3dTicks {
void GetTime(struct timespec* ts)
{
#ifdef URG3D_MAC_OS // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;

#else
#ifndef URG3D_WINDOWS_OS
    clock_gettime(CLOCK_REALTIME, ts);
#endif
#endif
}

int64_t Urg3dTicksMs(void)
{
    static int isInitialized = 0;
#if defined(URG3D_WINDOWS_OS)
    clock_t currentClock;
#else
    static struct timespec firstSpec;
    struct timespec currentSpec;
#endif
    long timeMs;

#if defined(URG3D_WINDOWS_OS)
    if (isInitialized == 0) {
        isInitialized = 1;
    }
    currentClock = clock();
    timeMs = currentClock / (CLOCKS_PER_SEC / 1000);
#else
    if (isInitialized == 0) {
        Urg3dTicks::GetTime(&firstSpec);
        isInitialized = 1;
    }
    Urg3dTicks::GetTime(&currentSpec);
    timeMs =
            (currentSpec.tv_sec - firstSpec.tv_sec) * 1000
            + (currentSpec.tv_nsec - firstSpec.tv_nsec) / 1000000;
#endif
    return timeMs;
}

}

#endif
