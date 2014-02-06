// ============================================================================
#include "errorFlipBits.h"

#include <stdio.h>
#include <arpa/inet.h>
// ============================================================================
static const char * __classname = "errorFlipBits";
// ============================================================================
int errorFlipBits::run(void** pBuf, size_t* pLen, uint32_t msgNo, bool isSend)
{
    if ((pBuf == NULL) || (*pBuf == NULL))
    {
        ERR_PRINT("NULL Pointer\n"); 
        return -1;
    }
    
    uint32_t seqNo = ntohl(*(uint32_t*)(*pBuf));
    (void)(seqNo);
    MSG_PRINT("MSG# %3u SEQ# %3u - FLIPPED BITS\n", msgNo, seqNo); 

    double d_len = *pLen;
    int byte_to_flip = (int)(d_len * drand48());

    ((uint8_t*)*pBuf)[byte_to_flip] ^= 0xFF;

    return 1;
}
// ============================================================================
int errorFlipBits::report(void)
{
    return 0;
}
// ============================================================================
const char* errorFlipBits::getName(void)
{
    return __classname;
}
// ============================================================================
// ============================================================================
