/* 
 * CPE464 Library - checksum
 *
 * Checksum declaration 
 * shadows@whitefang.com
 *
 * Simple call in_cksum with a memory location and it will calculate
 * the checksum over the requested length. The results are turned in 
 * a 16-bit, unsigned short
 */

#ifdef __cplusplus
extern "C" {
#endif

unsigned short in_cksum(unsigned short *addr, int len);

#ifdef __cplusplus
}
#endif

/*
 * CPE464 Library - Hooks for the following: bind, select, send, sendto
 *
 * Identical usage as the original functions, but allows the grader to
 * check/change the actions the call does.
 */

#ifndef _CPE464_HOOKS_H_
#define _CPE464_HOOKS_H_

// ============================================================================
#define DEBUG_ON  1
#define DEBUG_OFF 0

#define FLIP_ON   1
#define FLIP_OFF  0

#define DROP_ON   1
#define DROP_OFF  0

#define RSEED_ON  1
#define RSEED_OFF 0
// ============================================================================
// Options
#define CPE464_OVERRIDE_RECV
// ============================================================================

#ifdef __cplusplus
extern "C" {
#endif

    // Bind
    #include <sys/types.h>
    #include <sys/socket.h>

    // Select
    #include <sys/select.h>

    int bindMod(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int selectMod(int nfds,
                  fd_set *readfds,
                  fd_set *writefds,
                  fd_set *exceptfds,
                  struct timeval *timeout);
    /*
     * In your main()
     *
     * To use the sendErr(...) function you must call sendErr_init(...) first.
     * sendErr_init(...) initializes fields that are used by sendErr(...).  You
     * should only call sendErr_init(...) once in each program.
     * 
     * sendErr_init(...)
     *    error_rate  - between 0 and less than 1 (0 means no errors)
     *    drop_flag   - should packets be dropped (DROP_ON or DROP_OFF)
     *    flip_flag   - should bits be flipped (FLIP_ON or FLIP_OFF)
     *    debug_flag  - print out debug info (DEBUG_ON or DEBUG_OFF)
     *    random_flag - if set to RSEED_ON, then seed is randon, if RSEED_OFF then seed is not random
     *                  (see man srand48 - RSEED_OFF makes your program runs more repeatable)
     *
     *    If you don't know what to set random_flag to, use RSEED_OFF for initial
     *    debugging.  Once your program works, try RSEED_ON.  This will make the
     *    drop/flip pattern random between program runs.
     *    
     *    ex:
     *        sendErr_init(.1, DROP_ON, FLIP_OFF, DEBUG_ON, RSEED_ON);
     */

    int sendErr_init(double error_rate,
                     int drop_flag,
                     int flip_flag,
                     int debug_flag,
                     int random_flag);

    ssize_t sendErr  (int s, void *msg, int len, unsigned int flags);

    ssize_t recvErr(int s, void *buf, size_t len, int flags);

    ssize_t sendtoErr(int s, void *msg, int len, unsigned int flags,
                        const struct sockaddr *to, int tolen);
    
    ssize_t recvfromErr(int s, void *buf, size_t len, int flags,
                        struct sockaddr *from, socklen_t *fromlen);

    #define bind(...)     bindMod(__VA_ARGS__)
    #define select(...)   selectMod(__VA_ARGS__)

    #define send(...)     sendErr(__VA_ARGS__)
    #define sendto(...)   sendtoErr(__VA_ARGS__)

#ifdef CPE464_OVERRIDE_RECV
    #define recv(...)     recvErr(__VA_ARGS__)
    #define recvfrom(...) recvfromErr(__VA_ARGS__)
#endif

    #define sendtoErr_init(...) sendErr_init(__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif

