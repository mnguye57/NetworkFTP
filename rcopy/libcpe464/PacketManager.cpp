#include "PacketManager.h"

// ============================================================================
// Since we overrode send(to) functions, we need to undef here (before re-inc)

#ifdef send
    #undef send
#endif

#ifdef sendto
    #undef sendto
#endif
// ============================================================================
#include <stdint.h>
#include <stdio.h>

#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <string.h>

#include <arpa/inet.h>
// ============================================================================
PacketManager::PacketManager() :
    m_ErrorRate(0.0f), m_MsgNo(0)
{
    srand48(time(NULL));
}
// ============================================================================
PacketManager::~PacketManager()
{
    clearMsgEvents(m_ErrorCase_Constant);
    clearMsgEvents(m_ErrorCase_Chance);
}
// ============================================================================
int PacketManager::clearMsgEvents(listMsgEvents_t& ErrVec)
{
    int count = 0;

    while (ErrVec.size() > 0)
    {
        IMsgEvent* pErrCase = ErrVec.back();
        ErrVec.pop_back();
        delete pErrCase;
        ++count;
    }

    return count;
}
// ============================================================================
int PacketManager::setRandSeed(long seed)
{
    srand48(seed);

    return 0;
}
// ============================================================================
int PacketManager::setErrorRate(float rate)
{
    m_ErrorRate = rate;

    return 0;
}
// ============================================================================
int PacketManager::addMsgEvent_Standard(IMsgEvent* msgErr)
{
    if (msgErr == NULL)
    {
        return -1;
    }

    m_ErrorCase_Constant.push_back(msgErr);

    return 0;
}
// ============================================================================
int PacketManager::addMsgEvent_Random(IMsgEvent* msgErr)
{
    if (msgErr == NULL)
    {
        return -1;
    }

    m_ErrorCase_Chance.push_back(msgErr);

    return 0;
}
// ============================================================================
int PacketManager::runMsgEvents(listMsgEvents_t& ErrVec, void** pBuf, size_t* pLen, uint32_t msgNo)
{
    if ((pBuf == NULL) || (*pBuf == NULL))
    {
        ERR_PRINT("NULL Pointer\n");
        return -1;
    }

    int nResult;
    bool hasChanged = false;

    for (uint i = 0; i < ErrVec.size(); ++i)
    {
        nResult = ErrVec[i]->run(pBuf, pLen, msgNo);
        if (nResult < 0)
        {
            ERR_PRINT("ErrorCase Run '%s' Failed", ErrVec[i]->getName());
            return -1;
        }
        else if (nResult == 1)
        {
            hasChanged = true;
        }
        else if (nResult == 2)
        {
            return 2;
        }
    }

    return hasChanged;
}
// ============================================================================
int PacketManager::processEvents(void** pBuf, size_t* pLen, uint32_t msgNo)
{
    if ((pBuf == NULL) || (*pBuf == NULL))
    {
        ERR_PRINT("NULL Pointer\n");
        return -1;
    }

    int nResult = 0;
    bool hasChanged = false;
    bool hasDropped = false;

    nResult = runMsgEvents(m_ErrorCase_Constant, pBuf, pLen, msgNo);
    if (nResult < 0)
    {
        return nResult;
    }
    else if (nResult == 2)
    {
        hasDropped = true;
    }
    else if (nResult == 1)
    {
        hasChanged = true;
    }

    // No random errors on the first packet
    if (msgNo > 1)
    {
      // Decide (based on error rate) if we should produce an chance error
      float randNum = drand48();
      if ((m_ErrorCase_Chance.size() > 0) && (randNum <= m_ErrorRate))
      {
          // Chose which one to run
          int randCase = (int)((float)m_ErrorCase_Chance.size() * drand48());
          nResult = m_ErrorCase_Chance[randCase]->run(pBuf, pLen, msgNo);
          if (nResult < 0)
          {
              return nResult;
          }
          else if (nResult == 2)
          {
              hasDropped = true;
          }
          else
          {
              hasChanged = nResult;
          }
      }
    }

    if (hasDropped)
    {
        return 2;
    }
    else
    {
        return hasChanged;
    }
}
// ============================================================================
ssize_t PacketManager::send_Err(int s, void *buf, size_t len, int flags)
{
    ssize_t nResult = 0;

    if (buf == NULL)
    {
        ERR_PRINT("buf pointer == NULL\n");
        exit(1);
    }

    if (len == 0)
    {
        ERR_PRINT("len == 0: %u\n", len);
        exit(1);
    }

    ++m_MsgNo;
    
    uint32_t seqNo = ntohl(*(uint32_t*)(buf));
    MSG_PRINT("MSG# %3u SEQ# %3u LEN %4u FLAGS 0x%08X\n", m_MsgNo, seqNo, len, flags); 
    
    size_t lenTmp = len;
    unsigned char bufTmp[len];
    memcpy(bufTmp, buf, lenTmp);
    void* pBuf = bufTmp;

    nResult = processEvents((void**)&pBuf, &lenTmp, m_MsgNo);
    // Error Case
    if (nResult < 0)
    {
        // Do nothing. Will return nResult
    }
    // (Non-)changed Cases
    else if ((nResult == 0) || (nResult == 1))
    {
        ssize_t lenSent = send(s, bufTmp, lenTmp, flags);
        if (lenSent == (ssize_t)lenTmp)
        {
            nResult = len;
        }
        else
        {
            nResult = lenSent;
        }
    }
    // Drop Case
    else
    {
        nResult = len;
    }

    return nResult;
}
// ============================================================================
ssize_t PacketManager::recv_Mod(int s, void *buf, size_t len, int flags)
{
    ssize_t ret = ::recv(s, buf, len, flags);
    
    uint32_t seqNo = ntohl(*(uint32_t*)(buf));
    MSG_PRINT("         SEQ# %3u LEN %4u FLAGS 0x%08X\n", seqNo, ret, flags);

    return ret;
}
// ============================================================================
ssize_t PacketManager::sendto_Err(int s, void *buf, size_t len, int flags,
                                  const struct sockaddr *to, socklen_t tolen)
{
    int nResult = 0;

    if (buf == NULL)
    {
        ERR_PRINT("buf pointer == NULL\n");
        exit(1);
    }

    if (len == 0)
    {
        ERR_PRINT("len == 0: %u\n", len);
        exit(1);
    }
    
    if (to == NULL)
    {
        ERR_PRINT("sockaddr pointer == NULL\n");
        exit(1);
    }

    ++m_MsgNo;

    uint32_t seqNo = ntohl(*(uint32_t*)(buf));
    MSG_PRINT("MSG# %3u SEQ# %3u LEN %4u FLAGS 0x%08X\n", m_MsgNo, seqNo, len, flags); 
    
    size_t lenTmp = len;
    unsigned char bufTmp[len];
    memcpy(bufTmp, buf, lenTmp);
    void* pBuf = bufTmp;

    nResult = processEvents((void**)&pBuf, &lenTmp, m_MsgNo);
    if (nResult < 0)
    {
        ERR_PRINT("prcoessEvents\n");
        return nResult;
    }
    else if ((nResult == 0) || (nResult == 1))
    {
        ssize_t lenSent = sendto(s, pBuf, lenTmp, flags, to, tolen);
        if (lenSent == (ssize_t)lenTmp)
        {
            return len;
        }
        else
        {
            return lenSent;
        }
    }
    else
    {
        return len;
    }

    return -1;
}
// ============================================================================
ssize_t PacketManager::recvfrom_Mod(int s, void *buf, size_t len, int flags,
                   struct sockaddr *from, socklen_t *fromlen)
{
    ssize_t ret = ::recvfrom(s, buf, len, flags, from, fromlen);

    uint32_t seqNo = ntohl(*(uint32_t*)(buf));
    MSG_PRINT("         SEQ# %3u LEN %4u FLAGS 0x%08X\n", seqNo, ret, flags);

    return ret;
}
// ============================================================================
// ============================================================================
