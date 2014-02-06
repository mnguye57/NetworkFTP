/**
 * SettingsManager - Handles importing & processing all environmental variables
 *
 * To provide a means to override compiled options, the class acts as a hub for
 * compiled and environment options. Should latter be defined, it will always
 * have precedence over any compiled option.
 *
 * Environmental Variables (and value options):
 *   CPE464_AUTOGRADER          Current Unused
 *   CPE464_OVERRIDE_PORT       [0-65535] Override the port of first "bind(x)"
 *   CPE464_OVERRIDE_DEBUG      [-1 to 3] Sets debug level (ERR-VERBOSE)
 *   CPE464_OVERRIDE_SEEDRAND   [0-...]   Sets the seed value for random ops
 *   CPE464_OVERRIDE_ERR_RATE   [0.0-1.0] Percent error rate for random events
 *   CPE464_OVERRIDE_ERR_DROP   (see list detail below)
 *   CPE464_OVERRIDE_ERR_FLIP   (see list detail below)
 *
 * List Options:
 *   Provide a comma-separated list of MsgEvents to perform an event. Since no
 *   parameter undefines an environmental variable, use -1 to set random events
 */

#ifndef __SETTINGSMANAGER_H_
#define __SETTINGSMANAGER_H_

// ============================================================================
#include "PacketManager.h"
#include <list>
#include <map>
// ============================================================================
enum eEnvData_t
{
    EDT_UNKNOWN = 0,
    EDT_LONG,
    EDT_FLOAT,
    EDT_BOOL,
    EDT_CHARPTR,
    EDT_LIST_LONG
};

enum eEnvDataKey_t
{
    EDK_UNKNOWN = 0,
    EDK_AUTOGRADER,
    EDK_OVERRIDE_PORT,
    EDK_OVERRIDE_DEBUG,
    EDK_OVERRIDE_SEEDRAND,
    EDK_OVERRIDE_ERR_RATE,
    EDK_OVERRIDE_ERR_DROP,
    EDK_OVERRIDE_ERR_FLIP
};

typedef std::list<long> ListLong_t;

typedef struct _EnvDataEntry
{
    eEnvDataKey_t key;
    eEnvData_t    type;
    bool          isSet;
    union
    {
        long  vLong;
        float vFloat;
        bool  vBool;
        char* vCharPtr;
    } data;
    ListLong_t lLong;

    _EnvDataEntry() {
        isSet = false;
    };
} sEnvDataEntry_t;

typedef struct _EnvKeyInfo
{
    eEnvDataKey_t key;
    const char*   keyStr;
    eEnvData_t    type;
} sEnvKeyInfo_t;
// ============================================================================

class SettingsManager
{
    public:
    // ====================================================================
        SettingsManager(PacketManager& pktMgr);
        ~SettingsManager();

        int setUserMode_Debug(int debugLevel);
        int setUserMode_SeedRand(long seedValue);

        int setUserMode_ErrRate(float rate);
        int setUserMode_ErrDrop(bool isEnabled);
        int setUserMode_ErrFlip(bool isEnabled);
        // ====================================================================

    private:
        // ===== Helper Functions =============================================
        int parser2ListLong(ListLong_t& lLong, const char* str);
        int parserLong2Uint32(ListLong_t& lLong, std::list<uint32_t>& lUint32);

        // ====================================================================
        int loadEnvData(void);
        int loadEnvData_Port(void);
        int loadEnvData_Autograder(void);
        int loadEnvData_Debug(void);
        int loadEnvData_SeedRand(void);
        int loadEnvData_ErrRate(void);
        int loadEnvData_ErrDrop(void);
        int loadEnvData_ErrFlip(void);

        // ====================================================================
        typedef std::map<eEnvDataKey_t, sEnvDataEntry_t> sEnvDataMap_t;

        PacketManager* m_pPktMgr;
        sEnvDataMap_t  m_EnvData;
        // ====================================================================
};

#endif
