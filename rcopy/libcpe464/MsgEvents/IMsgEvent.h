/**
 * IMsgEvent - An interface for all event modules which derive from
 *
 * Within the interface, there are three functions:
 *   run     - takes in a buffer and can modify it. (<0 Err, 0 No-Chg, >0 Chg)
 *   report  - provides a summary of the events
 *   getName - returns a string of the object name
 */

#ifndef __IMSGEVENT_H
#define __IMSGEVENT_H

// ============================================================================
#include <stdlib.h>
#include <stdint.h>

#include "../utils/dbg_print.h"
// ============================================================================
#define MSG_PRINT_LEVEL DBG_LEVEL_INFO
#define MSG_PRINT(FMT, ...) DBG_PRINT(MSG_PRINT_LEVEL, "Msg - " FMT , ##__VA_ARGS__);
// ============================================================================
class IMsgEvent
{
	public:
		virtual ~IMsgEvent() {};

    /**
     * Function to be called when running the event case.
     *
     * Since the function can modify or drop data, a pointer
     * to the buf* make it possible to re-create a buffer and
     * change the len accordingly.
     *
     * Return Values:
     *   <0  Error
     *    0  No change
     *    1  Change
     *    2  Drop Completely
     */
    virtual int run(void** pBuf, size_t* pLen, uint32_t msgNo, bool isSend = true) = 0;

    virtual int report(void) = 0;

    virtual const char* getName(void) = 0;
};
// ============================================================================

#endif
