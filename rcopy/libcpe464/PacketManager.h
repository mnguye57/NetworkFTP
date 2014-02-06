/**
 * PacketManager - Handles all packet send and receive operations
 *
 * This class provides the mechanism to have MsgEvents (such as errors creation
 * and info collection) to be run on each or random packets. When each packet
 * is sent, all "Standard" MsgEvents will be performed. Additionally, there is
 * a random chance for "Random" events to happen.
 *
 * For event tracking (such as seqNo printing), the receive functions are also
 * processed through this class. Currently MsgEvents have no affect on the
 * receive functions (however, this may be added later to provide info event
 * processing.)
 */

#ifndef __PACKETMANAGER_H
#define __PACKETMANAGER_H

#include "MsgEvents/IMsgEvent.h"

#include <sys/socket.h>
#include <vector>

class PacketManager
{
  public:
    typedef std::vector<IMsgEvent*> listMsgEvents_t;

    PacketManager();
    ~PacketManager();

    int setRandSeed(long seed);
    int setErrorRate(float rate);

    int addMsgEvent_Standard(IMsgEvent* errorCase);
    int addMsgEvent_Random(IMsgEvent* errorCase);

    int processEvents(void** pBuf, size_t* pLen, uint32_t msgNo);

    ssize_t send_Err(int s, void *buf, size_t len, int flags);

    ssize_t recv_Mod(int s, void *buf, size_t len, int flags);

    ssize_t sendto_Err(int s, void *buf, size_t len, int flags,
                   const struct sockaddr *to, socklen_t tolen);

    ssize_t recvfrom_Mod(int s, void *buf, size_t len, int flags,
                   struct sockaddr *from, socklen_t *fromlen);

  private:
    float      m_ErrorRate;
    uint32_t   m_MsgNo;

    listMsgEvents_t m_ErrorCase_Constant;
    listMsgEvents_t m_ErrorCase_Chance;

    int runMsgEvents(listMsgEvents_t& ErrVec, void** pBuf, size_t* pLen, uint32_t msgNo);

    int clearMsgEvents(listMsgEvents_t& ErrVec);
};

#endif
