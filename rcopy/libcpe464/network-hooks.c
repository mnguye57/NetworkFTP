// ============================================================================
#include "networks/network-hooks.h"

#include "PacketManager.h"
#include "SettingsManager.h"

#include "MsgEvents/errorDrop.h"
#include "MsgEvents/errorFlipBits.h"
#include "MsgEvents/infoSeqNo.h"

#include "utils/dbg_print.h"
// ============================================================================
// Since we overrode send(to) functions, we need to undef here (before re-inc)

#undef bind
#undef select
#undef send
#undef sendto

#ifdef CPE464_OVERRIDE_RECV
    #undef recv
    #undef recvfrom
#endif
// ============================================================================
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <arpa/inet.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
// ============================================================================
PacketManager   g_PktMgr;
SettingsManager g_SetsMgr(g_PktMgr);
// ============================================================================
int bindMod(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    static int s_hasOverridden = 0;
    int port;

    char* env_port;
    env_port = getenv("CPE464_OVERRIDE_PORT");

    if (env_port && !s_hasOverridden)
    {
        s_hasOverridden = 1;
        port = atol(env_port);

        DBG_PRINT(DBG_LEVEL_WARN, "Port Override - %i\n", port);

        ((struct sockaddr_in*)addr)->sin_port = htons(port);
    }

    int nResult = bind(sockfd, addr, addrlen);

    struct sockaddr_in addr_in;
    socklen_t addr_in_len;

    getsockname(sockfd, (sockaddr*)&addr_in, &addr_in_len);
    port = ntohs(addr_in.sin_port);

    DBG_PRINT(DBG_LEVEL_DEBUG, "Port %i\n", port);

    return nResult;
}
// ============================================================================
int selectMod(int nfds,
              fd_set *readfds,
              fd_set *writefds,
              fd_set *exceptfds,
              struct timeval *timeout)
{

    if (timeout == NULL)
    {
        DBG_PRINT(DBG_LEVEL_INFO, "NULL timeout... infinite wait\n");
    }
    else
    {
        DBG_PRINT(DBG_LEVEL_INFO, "Secs %2li, uSecs %2li\n", timeout->tv_sec, timeout->tv_usec);
    }

    int nResult = select(nfds, readfds, writefds, exceptfds, timeout);

    if (nResult < 0)
    {
        DBG_PRINT(DBG_LEVEL_DEBUG, " >> Result <0: %s\n", strerror(errno));
    }
    else if (nResult == 0)
    {
        DBG_PRINT(DBG_LEVEL_DEBUG, " >> Result =0: Timed Out\n");
    }
    else
    {
        DBG_PRINT(DBG_LEVEL_DEBUG, " >> Result >0: Data Waiting\n");
    }

    return nResult;
}
// ============================================================================
int sendErr_init(double error_rate,
                 int drop_flag,
                 int flip_flag,
                 int debug_flag,
                 int random_flag)
{
    DBG_PRINT(DBG_LEVEL_VDEBUG, "\n");
            
    g_PktMgr.addMsgEvent_Standard(new infoSeqNo());

    g_SetsMgr.setUserMode_ErrRate(error_rate);
    g_SetsMgr.setUserMode_ErrDrop(drop_flag);
    g_SetsMgr.setUserMode_ErrFlip(flip_flag);
    g_SetsMgr.setUserMode_SeedRand(random_flag);
    g_SetsMgr.setUserMode_Debug(debug_flag);

    return 0;
}
// ============================================================================
ssize_t sendErr  (int s, void *msg, int len, unsigned int flags)
{
    //DBG_PRINT(DBG_LEVEL_VDEBUG, "\n");

    return g_PktMgr.send_Err(s, msg, len, flags);
}
// ============================================================================
ssize_t recvErr(int s, void *buf, size_t len, int flags)
{
    //DBG_PRINT(DBG_LEVEL_VDEBUG, "\n");

    return g_PktMgr.recv_Mod(s, buf, len, flags);
}
// ============================================================================
ssize_t sendtoErr(int s, void *msg, int len, unsigned int flags,
              const struct sockaddr *to, int tolen)
{
    //DBG_PRINT(DBG_LEVEL_VDEBUG, "\n");

    return g_PktMgr.sendto_Err(s, msg, len, flags, to, tolen);
}
// ============================================================================
ssize_t recvfromErr(int s, void *buf, size_t len, int flags,
              struct sockaddr *from, socklen_t *fromlen)
{
    //DBG_PRINT(DBG_LEVEL_VDEBUG, "\n");

    return g_PktMgr.recvfrom_Mod(s, buf, len, flags, from, fromlen);
}
// ============================================================================
// ============================================================================
