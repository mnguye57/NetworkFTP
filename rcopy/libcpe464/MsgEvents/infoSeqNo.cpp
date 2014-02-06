// ============================================================================
#include "infoSeqNo.h"

#include <stdio.h>
#include <arpa/inet.h>
// ============================================================================
static const char * __classname = "infoSeqNo";
// ============================================================================
infoSeqNo::infoSeqNo() :
    m_ValidEndian(true)
{
}
// ============================================================================
infoSeqNo::~infoSeqNo()
{
    this->report();
}
// ============================================================================
int infoSeqNo::run(void** pBuf, size_t* pLen, uint32_t msgNo, bool isSend)
{
    if ((pBuf == NULL) || (*pBuf == NULL))
    {
        ERR_PRINT("NULL Pointer\n");
        return -1;        
    }

    uint32_t seqNo = ntohl(*(uint32_t*)(*pBuf));

    // TODO check if seqNo is likely correct or flipped
    
    //MSG_PRINT("MSG# %u SEQ# %u\n", msgNo, seqNo); 

    m_History.push_back(seqNo);

    ++m_Count[seqNo];

    return 0;
}
// ============================================================================
int infoSeqNo::report(void)
{
    fprintf(stderr, "======== SeqNo Report ========\n");
    fprintf(stderr, "  Msgs (Total)       : %5u\n", m_History.size());
    fprintf(stderr, "  Msgs (Unique SeqNo): %5u\n", m_Count.size());
    fprintf(stderr, "==============================\n");

    return 0;
}
// ============================================================================
const char* infoSeqNo::getName(void)
{
    return __classname;
}
// ============================================================================
// ============================================================================
