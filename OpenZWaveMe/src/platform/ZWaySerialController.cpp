#include "ZWaySerialController.h"

namespace OpenZWaveMe
{
    ZWaySerialController::ZWaySerialController() :
        m_lastError(NoError)
    {
        memset(&m_zway, 0, sizeof(ZWay));
    }
    //----------------------------------------------------------------------------------------------------

    ZWaySerialController::~ZWaySerialController()
    {
    }
    //----------------------------------------------------------------------------------------------------

    bool ZWaySerialController::Open(string const& _serialControllerName)
    {
        m_lastError = zway_init(&m_zway, _serialControllerName.c_str(), ZSTR("config"), ZSTR("translations"), ZSTR("ZDDX"), NULL, NULL);

        if (m_lastError != NoError)
        {
            OZW_FATAL_ERROR(m_lastError, "Failed to init ZWay");
        }

        m_lastError = zway_start(m_zway, NULL, NULL);

        if (m_lastError != NoError)
        {
            OZW_FATAL_ERROR(m_lastError, "Failed to start ZWay");
        }

        return !(bool)m_lastError;
    }
    //----------------------------------------------------------------------------------------------------

    bool ZWaySerialController::Close()
    {
        m_lastError = zway_stop(m_zway);

        zway_terminate(&m_zway);

        return !(bool)m_lastError;
    }
    //---------------------------------------------------------------------------------------------------

    bool ZWaySerialController::IsRunning()
    {
        return zway_is_running(m_zway);
    }
    //---------------------------------------------------------------------------------------------------

    bool ZWaySerialController::IsIdle()
    {
        return zway_is_idle(m_zway);
    }
    //---------------------------------------------------------------------------------------------------

    uint32 ZWaySerialController::Write(uint8* _buffer, uint32 _length)
    {
        return 0;
    }
    //----------------------------------------------------------------------------------------------------

    void ZWaySerialController::PlayInitSequence(Driver* _driver)
    {
        m_lastError = zway_discover(m_zway);

        if (m_lastError != NoError)
        {
            OZW_FATAL_ERROR(m_lastError, "Failed to negotiate with Z-Wave stick");
        }

        Log::Write(LogLevel::LogLevel_Info, "Z-Way controller discovered");
    }
    //----------------------------------------------------------------------------------------------------
}
