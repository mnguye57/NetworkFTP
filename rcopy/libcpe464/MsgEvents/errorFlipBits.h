/**
 * errorDrop - Drops specific (or all) packets passed in
 *
 * When called, this class will bitwise XOR a random byte within the packet
 */

#ifndef __MSGERROR_FLIPBITS_H
#define __MSGERROR_FLIPBITS_H

// ============================================================================
#include "IMsgEvent.h"
// ============================================================================
class errorFlipBits : public IMsgEvent
{
	public:
    errorFlipBits() {};
    virtual ~errorFlipBits() {};

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
    virtual int run(void** pBuf, size_t* pLen, uint32_t seqno, bool isSend);

    virtual int report(void);

    virtual const char* getName(void);
};
// ============================================================================

#endif

