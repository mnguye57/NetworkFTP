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

