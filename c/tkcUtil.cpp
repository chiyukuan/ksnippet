//                            +-----------------+
//----------------------------+ C++ SOURCE FILE +-----------------------------
//                            +-----------------+
/**
 * @file
 * $Id:$
 * $Date:$
 * @par Description:
 * The Description of this file.
 * @par Copyright:
 * Copyright (c) by Kodiak Data, Inc. All rights reserved.
 */

//----------------------------------------------------------------------------
//                                                                          --
// MODULES USED                                                             --
//                                                                          --
//----------------------------------------------------------------------------
#include "tkcBase.hpp"

#ifdef __MODULE_DECLARATIONS__
#ifndef __TKCUTIL_HPP__
#define __TKCUTIL_HPP__
//----------------------------------------------------------------------------
//                                                                          --
// DEFINITIONS AND MACROS                                                   --
//                                                                          --
//----------------------------------------------------------------------------
#if defined(__i386__)
#define TBHI 1          /* LITTLE ENDIAN */
#define TBLO 0

#define POLL_CLOCK() rdtsc()
#ifdef CHECK_BACKWARDS_TIME
static uint64_t orv = 0 ;
#endif
static inline uint64_t
rdtsc(void)
{
    uint64_t rv ;

    __asm __volatile(".byte 0x0f, 0x31" : "=A" (rv)) ;

#ifdef CHECK_BACKWARDS_TIME
    if (rv < orv) {
	printf("mftb: time moved backwards\n") ;
	exit(66) ;
    }
    orv = rv ;
#endif
    return rv ;
}
#elif defined(__x86_64__)
#define TBHI 1          /* LITTLE ENDIAN */
#define TBLO 0

#define POLL_CLOCK() rdtsc()
static inline uint64_t
rdtsc(void)
{
    unsigned hi, lo ;

    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi)) ;
    return ( (uint64_t) lo) | ( ((uint64_t) hi) << 32 ) ;
}
#endif /* ifdef __i386__ */

#define max(a,b) \
    ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })

#define tkcEvent_notice( _format_, E... ) { \
    kdEvent( _format_, ##E ) ; \
    kdLog_notice( _format_, ##E ) ; \
}
#define tkcEvent_err( _format_, E... ) { \
    kdEvent( _format_, ##E ) ; \
    kdLog_err( _format_, ##E ) ; \
}

#define tkcLog_fb( __, _format_, _argv_...) \
    __( "%*s%s (" _format_ ") {", (tkcLogIndent ++) * 2 , "", __FUNCTION__, ##_argv_ )

#define tkcLog_fe( __, _format_, _argv_...) \
    __( "%*s} // %s " _format_,  (-- tkcLogIndent) * 2, "", __FUNCTION__, ##_argv_ )

//----------------------------------------------------------------------------
//                                                                          --
// TYPEDEFS AND STRUCTURES                                                  --
//                                                                          --
//----------------------------------------------------------------------------
class TkcUtil {
private:
    static uint64_t getCPUSpeed(void) ;
public:
    static uint64_t tick_m ;
    static uint64_t tick_s ;
    static uint64_t tick_28s ;
    static uint64_t tick_ms ;
    static uint64_t tick_us ;
    static uint64_t logBurstTOut ;
    static int      logLevel ;
    static RC_t init(void) ;

    static void gdbCutPoint() {}
    static void hang() ;
    static void burstLog(uint32_t sec) {
        if (kdLogLevel != LOG_DEBUG) {
            logLevel = kdLogLevel ;
            kdLog_setLevel( LOG_DEBUG ) ;
        }
        uint64_t tout = POLL_CLOCK() + (tick_s * sec) ;
        if (logBurstTOut < tout) {
            logBurstTOut = tout ;
        }
    }

    INLINE static void pull(void) {
        if (logBurstTOut && POLL_CLOCK() > logBurstTOut) {
            kdLog_setLevel( logLevel ) ;
            logBurstTOut = 0 ;
        }
    }
} ;
//----------------------------------------------------------------------------
//                                                                          --
// EXPORTED VARIABLES                                                       --
//                                                                          --
//----------------------------------------------------------------------------
extern int32_t tkcLogIndent ;
//----------------------------------------------------------------------------
//                                                                          --
// EXPORTED FUNCTIONS                                                       --
//                                                                          --
//----------------------------------------------------------------------------
const char* msgSig_getName(uint16_t msgSig) ;
const char* kdtkProtType_getName(uint8_t pType) ;
const char* kdtkRC_getName(kdtkRC_t kdtkRC) ;
const char* tkcAction_getName( tkcAction_t tkcAction ) ;
#endif // End of __TKCUTIL_HPP__

#else
// Section ------------------  Module implementation -------------------------
#include <ctype.h>
//----------------------------------------------------------------------------
//                                                                          --
// PROTOTYPES OF LOCAL FUNCTIONS                                            --
//                                                                          --
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//                                                                          --
// EXPORTED VARIABLES                                                       --
//                                                                          --
//----------------------------------------------------------------------------
int32_t tkcLogIndent = 0 ;
//----------------------------------------------------------------------------
//                                                                          --
// GLOBAL VARIABLES                                                         --
//                                                                          --
//----------------------------------------------------------------------------
uint64_t TkcUtil::tick_m ;
uint64_t TkcUtil::tick_s ;
uint64_t TkcUtil::tick_28s ;
uint64_t TkcUtil::tick_ms ;
uint64_t TkcUtil::tick_us ;
uint64_t TkcUtil::logBurstTOut ;
int      TkcUtil::logLevel ;
//----------------------------------------------------------------------------
//                                                                          --
// EXPORTED FUNCTIONS                                                       --
//                                                                          --
//----------------------------------------------------------------------------
RC_t
TkcUtil::init()
{
    RC_t rc = RC_ERROR ;
    do {
        tick_s = getCPUSpeed() ;
        tick_us = (tick_s + (1000000 - 1)) / 1000000;           /* round up */
        tick_ms = (tick_s + (1000 - 1)) / 1000;                 /* round up */
        tick_m  = tick_s * 60 ;
        tick_28s = tick_s * 28 ;
        logBurstTOut = 0 ;
        logLevel     = LOG_DEBUG ;
        rc = RC_OK ;
    } while(0) ;
    return rc ;
}


const char*
kdtkProtType_getName(uint8_t pType)
{
    return TkcPSpec::getPSpec( pType )->name ;
}
const char*
kdtkRC_getName(kdtkRC_t kdtkRC)
{
    return kdtkRC == KDTK_RC__OK           ? "OK" :
           kdtkRC == KDTK_RC__ERROR        ? "Error" :
                                             "Unknow" ;
}

VENUM_DESC_toStrFunc(    MSG_SIG_,    msgSig_getName,    uint16_t )
KD_ENUM_DESC__toStrFunc( TKC_ACTION_, tkcAction_getName, tkcAction_t )

void
TkcUtil::hang()
{
    bool hangMe = true ;

    do {
        usleep( 1000000 ) ;
    } while(hangMe) ;
}
//----------------------------------------------------------------------------
//                                                                          --
// LOCAL FUNCTIONS                                                          --
//                                                                          --
//----------------------------------------------------------------------------
uint64_t
TkcUtil::getCPUSpeed(void)
{
    FILE *cpuInfo = 0 ;
    float fval ;
    int   found = 0, index = 0 ;
    char  buf[80] ;
    uint64_t cpuSpeed = 2999 ;  //< 2999 MHz

    do {
        if ((cpuInfo = fopen("/proc/cpuinfo","r")) == 0) {
            // using the default cpu speed.
            break ;
        }
        while(!(found || feof(cpuInfo))) {
            fgets(buf, sizeof(buf) - 1, cpuInfo) ;
            if (strstr(buf, "cpu MHz") == 0)  continue ;

            // read cpu speed
            while(!isdigit(buf[index])) index ++ ;  // get first digit
            sscanf(&buf[index], "%f\n", &fval) ;
            cpuSpeed = (uint64_t) (fval * 1000000) ;
            found = 1 ;
        }
    } while (0) ;
    if (cpuInfo)  fclose(cpuInfo) ;

    return cpuSpeed ;
}

//----------------------------------------------------------------------------
//                                                                          --
// EOF                                                                      --
//                                                                          --
//----------------------------------------------------------------------------
#endif // End of module implementation

