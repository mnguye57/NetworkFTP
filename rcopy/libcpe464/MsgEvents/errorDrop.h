/**
 * errorDrop - Drops specific (or all) packets passed in
 *
 * General use would have a errorDrop with DropAll for random cases
 * and a drop list should a specific sequence of drops to occur using
 * the standard list within the PacketManager
 */

#ifndef __MSGERROR_DROP_H
#define __MSGERROR_DROP_H

// ============================================================================
#include "IMsgEvent.h"

#include <stdint.h>
#include <list>
// ============================================================================
class errorDrop : public IMsgEvent
{
	public:
    typedef std::list<uint32_t> DropList_t;

    errorDrop();
    virtual ~errorDrop() {};

    int setDropAll(bool dropAll);

    int setDropSpecific(DropList_t& dropList);

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

  private:
    bool       m_DropAll;
    DropList_t m_DropList;
};

#endif

