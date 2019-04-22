#include "Manager.h"

namespace OpenZWaveMe
{
    Manager* Manager::s_instance = NULL;

    Manager::Manager() :
        m_notificationMutex(new Mutex())
    {
        bool logging            = FALSE;
        bool bAppend            = FALSE;
        bool bConsoleOutput     = TRUE;
        int32 nSaveLogLevel     = LogLevel::LogLevel_Detail;
        int32 nQueueLogLevel    = LogLevel::LogLevel_Debug;
        int32 nDumpTrigger      = LogLevel::LogLevel_Warning;
        string userPath         = "";
        string logFileNameBase  = "OZW_Log.txt";

        // Ensure the singleton instance is set
        s_instance = this;

        // Create the log file (if enabled)
        Options::Get()->GetOptionAsBool("Logging", &logging);
        Options::Get()->GetOptionAsString("UserPath", &userPath);
        Options::Get()->GetOptionAsString("LogFileName", &logFileNameBase);
        Options::Get()->GetOptionAsBool("AppendLogFile", &bAppend);
        Options::Get()->GetOptionAsBool("ConsoleOutput", &bConsoleOutput);
        Options::Get()->GetOptionAsInt("SaveLogLevel", &nSaveLogLevel);
        Options::Get()->GetOptionAsInt("QueueLogLevel", &nQueueLogLevel);
        Options::Get()->GetOptionAsInt("DumpTriggerLevel", &nDumpTrigger);

        if (!nSaveLogLevel || nSaveLogLevel > LogLevel::LogLevel_StreamDetail)
        {
            Log::Write(LogLevel::LogLevel_Warning, "Invalid LogLevel Specified for SaveLogLevel in Options.xml");

            nSaveLogLevel = LogLevel::LogLevel_Detail;
        }

        if (!nQueueLogLevel || nQueueLogLevel > LogLevel::LogLevel_StreamDetail)
        {
            Log::Write(LogLevel::LogLevel_Warning, "Invalid LogLevel Specified for QueueLogLevel in Options.xml");

            nSaveLogLevel = LogLevel::LogLevel_Debug;
        }

        Log::Create(userPath + logFileNameBase, bAppend, bConsoleOutput, (LogLevel)nSaveLogLevel, (LogLevel)nQueueLogLevel, (LogLevel)nDumpTrigger);
        Log::SetLoggingState(logging);

        //CommandClasses::RegisterCommandClasses();
        //Scene::ReadScenes();

        //Log::Write(LogLevel::LogLevel_Always, "OpenZwave Version %s Starting Up", getVersionAsString().c_str());
    }
    //----------------------------------------------------------------------------------------------------

    Manager::~Manager()
    {
        for_each(m_pendingDrivers.begin(), m_pendingDrivers.end(), [](Driver* _driver) {delete _driver;});

        for_each(m_readyDrivers.begin(), m_readyDrivers.end(), [](pair<uint32, Driver*> _keyValue) {delete _keyValue.second;});
    }
    //----------------------------------------------------------------------------------------------------

    Manager* Manager::Create()
    {
        if (Options::Get() && Options::Get()->AreLocked())
        {
            if (NULL == s_instance)
            {
                s_instance = new Manager();
            }

            return s_instance;
        }

        // Options have not been created and locked.
        Log::Create("", FALSE, TRUE, LogLevel::LogLevel_Debug, LogLevel::LogLevel_Debug, LogLevel::LogLevel_None);

        Log::Write(LogLevel::LogLevel_Error, "Options have not been created and locked. Exiting...");

        //OZW_FATAL_ERROR(OZWException::OZWEXCEPTION_OPTIONS, "Options Not Created and Locked");

        return NULL;
    }
    //----------------------------------------------------------------------------------------------------

    void Manager::Destroy()
    {
        delete s_instance;

        s_instance = NULL;
    }
    //----------------------------------------------------------------------------------------------------

    void Manager::SetDriverReady(Driver* _driver, bool _success)
    {
    }
    //----------------------------------------------------------------------------------------------------

    void Manager::NotifyWatchers(Notification* _notification)
    {
        m_notificationMutex->Lock();

        //m_watcherIterators.push_back(&m_watchers.begin());

        for_each(m_watchers.begin(), m_watchers.end(), [&](Watcher* _watcher) {_watcher->m_callback(_notification, _watcher->m_context);});

        //m_watcherIterators.pop_back();

        m_notificationMutex->Unlock();
    }
    //----------------------------------------------------------------------------------------------------

    bool Manager::AddDriver(string const& _controllerPath, Driver::ControllerInterface _interface)
    {
        if (!m_pendingDrivers.empty() && any_of(m_pendingDrivers.begin(), m_pendingDrivers.end(), [&](Driver* _driver) {return _driver->GetControllerPath() == _controllerPath;}))
        {
            Log::Write(OpenZWave::LogLevel_Info, "mgr,     Cannot add driver for controller %s - driver already exists", _controllerPath.c_str());

            return FALSE;
        }

        if (!m_readyDrivers.empty() && any_of(m_readyDrivers.begin(), m_readyDrivers.end(), [&](pair<uint32, Driver*> _keyValue) {return _keyValue.second->GetControllerPath() == _controllerPath;}))
        {
            Log::Write(OpenZWave::LogLevel_Info, "mgr,     Cannot add driver for controller %s - driver already exists", _controllerPath.c_str());

            return FALSE;
        }

        Driver* driver = new Driver(_controllerPath, _interface);

        m_pendingDrivers.push_back(driver);

        driver->Start();

        Log::Write(OpenZWave::LogLevel_Info, "mgr,     Added driver for controller %s", _controllerPath.c_str());

        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------

    bool Manager::RemoveDriver(string const& _controllerPath)
    {
        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------

    bool Manager::AddWatcher(pfnOnNotification_t _watcher, void* _context)
    {
        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------
}
