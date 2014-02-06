// ============================================================================
#include "errorDrop.h"

#include <stdio.h>
#include <arpa/inet.h>
// ============================================================================
static const char * __classname = "errorDrop";
// ============================================================================
errorDrop::errorDrop() :
    m_DropAll(true)
{
}
// ============================================================================
int errorDrop::setDropAll(bool dropAll)
{
    m_DropAll = dropAll;

    return 0;
}
// ============================================================================
int errorDrop::setDropSpecific(DropList_t& dropList)
{
    m_DropAll = false;
    m_DropList = dropList;

    return 0;
}
// ============================================================================
int errorDrop::run(void** pBuf, size_t* pLen, uint32_t msgNo, bool isSend)
{
    if ((pBuf == NULL) || (*pBuf == NULL))
    {
        ERR_PRINT("NULL Pointer\n");
        return -1;        
    }

    bool toDrop = m_DropAll;

    DropList_t::iterator it = m_DropList.begin();
    while (it != m_DropList.end())
    {
        if (*it == msgNo)
        {
            toDrop = true;
            break;
        }
        ++it;
    }

    if (toDrop)
    {
        uint32_t seqNo = ntohl(*(uint32_t*)(*pBuf));
        (void)(seqNo);

        MSG_PRINT("MSG# %3u SEQ# %3u - DROPPED\n", msgNo, seqNo); 

        return 2;
    }
    else
    {
        return 0;
    }
}
// ============================================================================
int errorDrop::report(void)
{
    return 0;
}
// ============================================================================
const char* errorDrop::getName(void)
{
    return __classname;
}
// ============================================================================
// ============================================================================
