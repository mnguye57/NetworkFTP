// ============================================================================
#include "SettingsManager.h"

#include "utils/dbg_print.h"
#include "MsgEvents/errorDrop.h"
#include "MsgEvents/errorFlipBits.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
// ============================================================================
static sEnvKeyInfo_t g_EnvKeys[] = {
    {EDK_AUTOGRADER,        "CPE464_AUTOGRADER",        EDT_CHARPTR},
    {EDK_OVERRIDE_PORT,     "CPE464_OVERRIDE_PORT",     EDT_LONG},
    {EDK_OVERRIDE_DEBUG,    "CPE464_OVERRIDE_DEBUG",    EDT_LONG},
    {EDK_OVERRIDE_SEEDRAND, "CPE464_OVERRIDE_SEEDRAND", EDT_LONG},
    {EDK_OVERRIDE_ERR_RATE, "CPE464_OVERRIDE_ERR_RATE", EDT_FLOAT},
    {EDK_OVERRIDE_ERR_DROP, "CPE464_OVERRIDE_ERR_DROP", EDT_LIST_LONG},
    {EDK_OVERRIDE_ERR_FLIP, "CPE464_OVERRIDE_ERR_FLIP", EDT_LIST_LONG}
};
// ============================================================================
SettingsManager::SettingsManager(PacketManager& pktMgr) :
    m_pPktMgr(&pktMgr)
{
    dbg_setlevel(0);

    // load env settings
    loadEnvData();

    loadEnvData_Port();
    loadEnvData_Autograder();
    loadEnvData_Debug();
    loadEnvData_SeedRand();
    loadEnvData_ErrRate();
    loadEnvData_ErrDrop();
    loadEnvData_ErrFlip();
}
// ============================================================================
SettingsManager::~SettingsManager()
{
    // TODO iterate through EnvData for charPtrs and free
}
// ============================================================================
int SettingsManager::loadEnvData(void)
{
    uint EnvKeyInfo_Size = sizeof(g_EnvKeys) / sizeof(sEnvKeyInfo_t);
    for (uint i = 0; i < EnvKeyInfo_Size; ++i)
    {
        DBG_PRINT(DBG_LEVEL_DEBUG, "** Checking Env: %s **\n", g_EnvKeys[i].keyStr);

        char* tmpStr = getenv(g_EnvKeys[i].keyStr);
        //char* tmpStr = NULL;
        if (tmpStr != NULL)
        {
            DBG_PRINT(DBG_LEVEL_VDEBUG, "**** Found Env: %s => %s **\n", 
                    g_EnvKeys[i].keyStr, tmpStr);

            sEnvDataEntry_t& entry = m_EnvData[g_EnvKeys[i].key];
            entry.key   = g_EnvKeys[i].key;
            entry.type  = g_EnvKeys[i].type;
            entry.isSet = true;
            
            char* tmpStrEnd;
            switch (entry.type)
            {
                case EDT_LONG:
                {
                    entry.data.vLong = strtol(tmpStr, &tmpStrEnd, 10);
                    if (tmpStr == tmpStrEnd)
                    {
                        entry.isSet = false;
                        continue;
                    }
                    break;
                }
                case EDT_FLOAT:
                {
                    entry.data.vFloat = strtof(tmpStr, &tmpStrEnd);
                    if (tmpStr == tmpStrEnd)
                    {
                        entry.isSet = false;
                        continue;
                    }
                    break;
                }
                case EDT_BOOL:
                {
                    entry.data.vBool = strtol(tmpStr, &tmpStrEnd, 10);
                    if (tmpStr == tmpStrEnd)
                    {
                        entry.isSet = false;
                        continue;
                    }
                    break;
                }
                case EDT_CHARPTR:
                {
                    entry.data.vCharPtr = (char*)malloc(strlen(tmpStr));
                    if (entry.data.vCharPtr == NULL)
                    {
                        entry.isSet = false;
                        ERR_PRINT("malloc: %s", strerror(errno));
                        continue;
                    }

                    memcpy(entry.data.vCharPtr, tmpStr, strlen(tmpStr));
                    break;
                }
                case EDT_LIST_LONG:
                {
                    entry.data.vLong = parser2ListLong(entry.lLong, tmpStr);
                    if (entry.data.vLong < 0)
                    {
                        entry.isSet = false;
                        continue;
                    }
                    break;
                }
                default:
                {
                    continue;
                }
            }
        }
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_Autograder(void)
{
    if (m_EnvData[EDK_AUTOGRADER].isSet)
    {
        DBG_PRINT(DBG_LEVEL_WARN, "** ENV - Autograder **\n");
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_Port(void)
{
    if (m_EnvData[EDK_OVERRIDE_PORT].isSet)
    {
        // Do nothing... handled by bind
        DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE PORT: %li **\n",
                m_EnvData[EDK_OVERRIDE_PORT].data.vLong);
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_Debug(void)
{
    if (m_EnvData[EDK_OVERRIDE_DEBUG].isSet)
    {
        DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE DEBUG: %li **\n",
                m_EnvData[EDK_OVERRIDE_DEBUG].data.vLong);

        dbg_setlevel(m_EnvData[EDK_OVERRIDE_DEBUG].data.vLong);
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_SeedRand(void)
{
    if (m_EnvData[EDK_OVERRIDE_SEEDRAND].isSet)
    {
        DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE SEED RAND: %li **\n",
                m_EnvData[EDK_OVERRIDE_SEEDRAND].data.vLong);

        m_pPktMgr->setRandSeed(m_EnvData[EDK_OVERRIDE_SEEDRAND].data.vLong);
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_ErrRate(void)
{
    if (m_EnvData[EDK_OVERRIDE_ERR_RATE].isSet)
    {
        DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE ERROR RATE: %f **\n",
                m_EnvData[EDK_OVERRIDE_ERR_RATE].data.vFloat);

        m_pPktMgr->setErrorRate(m_EnvData[EDK_OVERRIDE_ERR_RATE].data.vFloat);
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_ErrDrop(void)
{
    sEnvDataEntry_t& entry = m_EnvData[EDK_OVERRIDE_ERR_DROP];
    if (entry.isSet)
    {
        errorDrop* errClass = new errorDrop();
        std::list<uint32_t> lUint32;
        parserLong2Uint32(entry.lLong, lUint32);

        if (lUint32.size() == 0)
        {
            DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE ERROR DROP: ENABLED **\n");

            m_pPktMgr->addMsgEvent_Random(errClass);
        }
        else
        {
            DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE ERROR DROP: __List__ **\n");
            for (std::list<uint32_t>::iterator it = lUint32.begin(); it != lUint32.end(); ++it)
            {
                DBG_PRINT(DBG_LEVEL_WARN, "%u\n", *it);
            }

            errClass->setDropSpecific(lUint32);
            m_pPktMgr->addMsgEvent_Standard(errClass);
        }
    }

    return 0;
}
// ============================================================================
int SettingsManager::loadEnvData_ErrFlip(void)
{
    sEnvDataEntry_t& entry = m_EnvData[EDK_OVERRIDE_ERR_FLIP];
    if (entry.isSet)
    {
        errorFlipBits* errClass = new errorFlipBits();
        std::list<uint32_t> lUint32;
        parserLong2Uint32(entry.lLong, lUint32);

        if (lUint32.size() == 0)
        {
            DBG_PRINT(DBG_LEVEL_WARN, "** ENV - OVERRIDE ERROR FLIP: ENABLED **\n");
            
            m_pPktMgr->addMsgEvent_Random(errClass);
        }
        else
        {
            // TODO implement
            //errClass->setDropSpecific(lUint32);
            //m_pPktMgr->addMsgEvent_Standard(errClass);
            return -1;
        }
    }
    return 0;
}
// ============================================================================
int SettingsManager::parserLong2Uint32(ListLong_t& lLong, std::list<uint32_t>& lUint32)
{
    ListLong_t::iterator it = lLong.begin();
    while (it != lLong.end())
    {
        if (*it >= 0)
        {
            lUint32.push_back(*it);
        }
        ++it;
    }

    return 0;
}
// ============================================================================
int SettingsManager::parser2ListLong(ListLong_t& lLong, const char* str)
{
    char* strTmp = (char*)str;
    char* pSave;

    int count = 0;

    char* token = strtok_r(strTmp, ",", &pSave);
    while (token != NULL)
    {
        char* strEnd = NULL;
        long val = strtol(token, &strEnd, 10);
        if (token != strEnd)
        {
            ++count;

            // Save Value
            lLong.push_back(val);

            // Get next value
            token = strtok_r(NULL, ",", &pSave);
        }
        else
        {
            ERR_PRINT("Invalid Value in String\n");
            break;
        }       
    }

    return count;
}
// ============================================================================
int SettingsManager::setUserMode_Debug(int debugLevel)
{
    if (m_EnvData[EDK_OVERRIDE_DEBUG].isSet)
    {
        return -1;
    }

    dbg_setlevel(debugLevel);

    return 0;
}
// ============================================================================
int SettingsManager::setUserMode_SeedRand(long seedValue)
{
    if (m_EnvData[EDK_OVERRIDE_SEEDRAND].isSet)
    {
        return -1;
    }

    return m_pPktMgr->setRandSeed(seedValue);
}
// ============================================================================
int SettingsManager::setUserMode_ErrRate(float rate)
{
    if (m_EnvData[EDK_OVERRIDE_ERR_RATE].isSet)
    {
        return -1;
    }

    return m_pPktMgr->setErrorRate(rate);
}
// ============================================================================
int SettingsManager::setUserMode_ErrDrop(bool isEnabled)
{
    if (m_EnvData[EDK_OVERRIDE_ERR_DROP].isSet)
    {
        return -1;
    }

    if (isEnabled)
    {
        return m_pPktMgr->addMsgEvent_Random(new errorDrop());
    }
    else
    {
        return 0;
    }
}
// ============================================================================
int SettingsManager::setUserMode_ErrFlip(bool isEnabled)
{
    if (m_EnvData[EDK_OVERRIDE_ERR_FLIP].isSet)
    {
        return -1;
    }

    if (isEnabled)
    {
        return m_pPktMgr->addMsgEvent_Random(new errorFlipBits());
    }
    else
    {
        return 0;
    }
}
// ============================================================================
// ============================================================================
