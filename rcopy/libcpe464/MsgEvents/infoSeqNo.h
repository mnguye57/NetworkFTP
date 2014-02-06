/**
 * infoSeqNo - maintains a log of all sequence numbers passed in
 *
 * No changes will be made to the buffers passed to this class, just count
 * the sequence numbers (which are assumed to be in the first 4-bytes of each
 * packet in network order.)
 *
 * Upon destruction, this class will call it's own report function in order.
 *
 * TODO: Report better details about the history instead of unique + count
 */

#ifndef __IMSGEVENT_SEQNO_H
#define __IMSGEVENT_SEQNO_H

// ============================================================================
#include "IMsgEvent.h"

#include <vector>
#include <map>
// ============================================================================
class infoSeqNo : public IMsgEvent
{
	public:
    typedef std::map<uint32_t, uint32_t> No2Count_t;
    typedef std::vector<uint32_t> NoJournal_t;

    infoSeqNo();
		virtual ~infoSeqNo();

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
    bool        m_ValidEndian;

    No2Count_t  m_Count;
    NoJournal_t m_History;
};
// ============================================================================

#endif
