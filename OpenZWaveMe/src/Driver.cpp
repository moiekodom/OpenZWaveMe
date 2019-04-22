#include "Manager.h"
#include "Driver.h"

namespace OpenZWaveMe
{
    Driver::Driver(string const& _controllerPath, ControllerInterface const& _interface) :
        m_bIntervalBetweenPolls(FALSE),
        m_waitingForAck(FALSE),
        m_awakeNodesQueried(FALSE),
        m_notifytransactions(FALSE),
        m_exit(FALSE),
        m_Controller_nodeId(0),
        m_expectedCallbackId(0),
        m_expectedReply(0),
        m_homeId(0),
        m_pollInterval(0),
        m_controllerPath(_controllerPath),
        m_controllerInterfaceType(_interface),
        m_currentControllerCommand(NULL),
        m_controller(NULL),
        m_notificationsEvent(new Event()),
        m_initMutex(new Mutex()),
        m_pollMutex(new Mutex()),
        m_nodeMutex(new Mutex()),
        m_driverThread(new Thread("driver")),
        m_pollThread(new Thread("poll")),
        m_currentMsg(NULL)
    {
        TimeStamp m_startTime;

        Options::Get()->GetOptionAsBool("NotifyTransactions", &m_notifytransactions);
        Options::Get()->GetOptionAsInt("PollInterval", &m_pollInterval);
        Options::Get()->GetOptionAsBool("IntervalBetweenPolls", &m_bIntervalBetweenPolls);

        // Create the message queue events
        for (uint8 i = 0; i < MsgQueue::MsgQueue_Count; ++i)
        {
            m_queueEvent[i] = new Event();
        }

        // Clear the nodes array
        memset(m_nodes, 0, sizeof(Node*) * 256);

        // Clear the virtual neighbors array
        memset(m_virtualNeighbors, 0, NUM_NODE_BITFIELD_BYTES);

        // Initilize the Network Keys
        InitNetworkKeys(FALSE);

        m_controller = new ZWaySerialController();

        m_controller->SetSignalThreshold(1);
    }
    //----------------------------------------------------------------------------------------------------

    Driver::~Driver()
    {
        m_controller->Close();
        m_controller->Release();
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::DriverThreadEntryPoint(Event* _exitEvent, void* _context)
    {
        Driver* driver = (Driver*)_context;

        if (driver)
        {
            driver->DriverThreadProc(_exitEvent);
        }
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::PollThreadEntryPoint(Event* _exitEvent, void* _context)
    {
        Driver* driver = (Driver*)_context;

        if (driver)
        {
            driver->PollThreadProc(_exitEvent);
        }
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::DriverThreadProc(Event* _exitEvent)
    {
        uint16 attempts     = 0;
        uint16 count        = 11;
        int32 maxAttempts   = 0;
        int32 timeout       = Wait::Timeout_Infinite;
        int32 retryTimeout  = RETRY_TIMEOUT;
        int32 res;
        TimeStamp retryTimeStamp;
        Wait* waitObjects[11];
        Notification* notification;

        Options::Get()->GetOptionAsInt("RetryTimeout", &retryTimeout);
        Options::Get()->GetOptionAsInt("DriverMaxAttempts", &maxAttempts);

        while (TRUE)
        {
            if (Init(attempts))
            {
                waitObjects[0]  = _exitEvent;				                    // Thread must exit.
                waitObjects[1]  = m_notificationsEvent;			                // Notifications waiting to be sent.
                waitObjects[2]  = m_controller;				                    // Controller has received data.
                waitObjects[3]  = m_queueEvent[MsgQueue::MsgQueue_Command];	    // A controller command is in progress.
                waitObjects[4]  = m_queueEvent[MsgQueue::MsgQueue_Security];	// Security Related Commands (As they have a timeout)
                waitObjects[5]  = m_queueEvent[MsgQueue::MsgQueue_NoOp];		// Send device probes and diagnostics messages
                waitObjects[6]  = m_queueEvent[MsgQueue::MsgQueue_Controller];	// A multi-part controller command is in progress
                waitObjects[7]  = m_queueEvent[MsgQueue::MsgQueue_WakeUp];		// A node has woken. Pending messages should be sent.
                waitObjects[8]  = m_queueEvent[MsgQueue::MsgQueue_Send];		// Ordinary requests to be sent.
                waitObjects[9]  = m_queueEvent[MsgQueue::MsgQueue_Query];		// Node queries are pending.
                waitObjects[10] = m_queueEvent[MsgQueue::MsgQueue_Poll];		// Poll request is waiting.

                while (TRUE)
                {
                    Log::Write(LogLevel::LogLevel_StreamDetail, "      Top of DriverThreadProc loop.");

                    // If we're waiting for a message to complete, we can only
                    // handle incoming data, notifications and exit events.
                    if (m_waitingForAck || m_expectedCallbackId || m_expectedReply)
                    {
                        count = 3;

                        timeout = m_waitingForAck ? ACK_TIMEOUT : retryTimeStamp.TimeRemaining();

                        if (timeout < 0)
                        {
                            timeout = 0;
                        }
                    }
                    else if (m_currentControllerCommand != NULL)
                    {
                        count = 7;
                    }
                    else
                    {
                        Log::QueueClear(); // clear the log queue when starting a new message
                    }

                    // Wait for something to do
                    res = Wait::Multiple(waitObjects, count, timeout);

                    switch (res)
                    {
                        case -1:

                            // Wait has timed out - time to resend
                            if (m_currentMsg != NULL)
                            {
                                notification = new Notification(NotificationType::Type_Notification);

                                notification->SetHomeAndNodeIds(m_homeId, m_currentMsg->GetTargetNodeId());
                                notification->SetNotification(NotificationCode::Code_Timeout);

                                QueueNotification(notification);
                            }

                            if (WriteMsg("Wait Timeout"))
                            {
                                retryTimeStamp.SetTime(retryTimeout);
                            }

                            break;
                        case 0:

                            // Exit has been signalled
                            return;
                        case 1:

                            // Notifications are waiting to be sent
                            NotifyWatchers();

                            break;
                        case 2:

                            // Data has been received
                            ReadMsg();

                            break;
                        default:

                            // All the other events are sending message queue items
                            if (WriteNextMsg((MsgQueue)(res - 3)))
                            {
                                retryTimeStamp.SetTime(retryTimeout);
                            }

                            break;
                    }
                }
            }

            ++attempts;

            if (maxAttempts && attempts >= maxAttempts)
            {
                Manager::Get()->Manager::SetDriverReady(this, FALSE);

                NotifyWatchers();

                break;
            }

            if (attempts < 25)
            {
                // Retry every 5 seconds for the first two minutes
                if (Wait::Single(_exitEvent, 5000) == 0)
                {
                    // Exit signalled.
                    return;
                }
            }
            else
            {
                // Retry every 30 seconds after that
                if (Wait::Single(_exitEvent, 30000) == 0)
                {
                    // Exit signalled.
                    return;
                }
            }
        }
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::PollThreadProc(Event* _exitEvent)
    {
        bool requestState;
        uint8 index;
        uint8 instance;
        int32 i32;
        int32 loopCount;
        int32 pollInterval;
        PollEntry pe;
        ValueID valueId;
        Value* value;
        Node* node;
        WakeUp* wakeUp;
        CommandClass* cc;

        while (TRUE)
        {
            pollInterval = m_pollInterval;

            if (!m_controller->IsRunning()) return;

            if (m_awakeNodesQueried && !m_pollList.empty())
            {
                // We only bother getting the lock if the pollList is not empty
                m_pollMutex->Lock();

                // Get the next value to be polled
                pe      = m_pollList.front();
                valueId = pe.m_id;

                m_pollList.pop_front();

                // Only execute this poll if pe.m_pollCounter == 1; otherwise decrement the counter and process the next polled value
                if (pe.m_pollCounter != 1)
                {
                    pe.m_pollCounter--;

                    m_pollList.push_back(pe);

                    m_pollMutex->Unlock();

                    continue;
                }

                // reset the poll counter to the full pollIntensity value and push it at the end of the list
                // release the value object referenced; call GetNode to ensure the node objects are locked during this period
                {
                    LockGuard LG(m_nodeMutex);

                    // (void)GetNode(valueId.GetNodeId()); // Для чего, не понятно. Если только в лог запись сделать.

                    value = GetValue(valueId);

                    if (!value) continue;

                    pe.m_pollCounter = value->GetPollIntensity();

                    m_pollList.push_back(pe);

                    value->Release();
                }

                // If the polling interval is for the whole poll list, calculate the time before the next poll,
                // so that all polls can take place within the user-specified interval.
                if (!m_bIntervalBetweenPolls)
                {
                    if (pollInterval < 100)
                    {
                        Log::Write(LogLevel::LogLevel_Info, "The pollInterval setting is only %d, which appears to be a legacy setting.  Multiplying by 1000 to convert to ms.", pollInterval);

                        pollInterval *= 1000;
                    }

                    pollInterval /= (int32)m_pollList.size();
                }

                {
                    LockGuard LG(m_nodeMutex);

                    // Request the state of the value from the node to which it belongs
                    if ((node = GetNode(valueId.GetNodeId())))
                    {
                        requestState = TRUE;

                        if (!node->IsListeningDevice())
                        {
                            // The device is not awake all the time.  If it is not awake, we mark it
                            // as requiring a poll.  The poll will be done next time the node wakes up.
                            if (wakeUp = static_cast<WakeUp*>(node->GetCommandClass(WakeUp::StaticGetCommandClassId())))
                            {
                                if (!wakeUp->IsAwake())
                                {
                                    wakeUp->SetPollRequired();

                                    requestState = FALSE;
                                }
                            }
                        }

                        if (requestState)
                        {
                            // Request an update of the value
                            CommandClass* cc = node->GetCommandClass(valueId.GetCommandClassId());

                            if (cc)
                            {
                                index       = valueId.GetIndex();
                                instance    = valueId.GetInstance();

                                Log::Write(LogLevel::LogLevel_Detail, node->m_nodeId, "Polling: %s index = %d instance = %d (poll queue has %d messages)", cc->GetCommandClassName().c_str(), index, instance, m_msgQueue[MsgQueue::MsgQueue_Poll].size());

                                cc->RequestValue(0, index, instance, MsgQueue::MsgQueue_Poll);
                            }
                        }

                    }
                }

                m_pollMutex->Unlock();

                // Polling messages are only sent when there are no other messages waiting to be sent
                // While this makes the polls much more variable and uncertain if some other activity dominates
                // a send queue, that may be appropriate
                // TODO we can have a debate about whether to test all four queues or just the Poll queue
                // Wait until the library isn't actively sending messages (or in the midst of a transaction)
                loopCount = 0;

                while (!m_msgQueue[MsgQueue::MsgQueue_Poll].empty() || !m_msgQueue[MsgQueue::MsgQueue_Send].empty() ||
                    !m_msgQueue[MsgQueue::MsgQueue_Command].empty() || !m_msgQueue[MsgQueue::MsgQueue_Query].empty() ||
                    m_currentMsg != NULL)
                {
                    i32 = Wait::Single(_exitEvent, 10);	// Test conditions every 10ms

                    if (i32 == 0)
                    {
                        // Exit has been called
                        return;
                    }

                    loopCount++;

                    if (loopCount == 3000 * 10) // 300 seconds worth of delay?  Something unusual is going on.
                    {
                        Log::Write(LogLevel::LogLevel_Warning, "Poll queue hasn't been able to execute for 300 secs or more");
                        Log::QueueDump();
                    }
                }

                // Ready for next poll...insert the pollInterval delay
                i32 = Wait::Single(_exitEvent, pollInterval);

                if (i32 == 0)
                {
                    // Exit has been called
                    return;
                }
            }
            else // Poll list is empty or awake nodes haven't been fully queried yet
            {
                // Don't poll just yet, wait for the pollInterval or exit before re-checking to see if the pollList has elements
                i32 = Wait::Single( _exitEvent, 500);

                if (i32 == 0)
                {
                    // Exit has been called
                    return;
                }
            }
        }
    }
    //----------------------------------------------------------------------------------------------------

    bool Driver::Init(uint32 _attempts)
    {
        m_initMutex->Lock();

        uint8 nak = NAK;

        m_Controller_nodeId = -1;
        m_waitingForAck     = FALSE;

        if (m_exit)
        {
            m_initMutex->Unlock();

            return FALSE;
        }

        // Open the controller
        Log::Write(LogLevel::LogLevel_Info, "  Opening controller %s", m_controllerPath.c_str());

        if (!m_controller->Open(m_controllerPath))
        {
            Log::Write(LogLevel::LogLevel_Warning, "WARNING: Failed to init the controller (attempt %d)", _attempts );

            m_initMutex->Unlock();

            return FALSE;
        }

        if (m_controllerInterfaceType != ControllerInterface_ZWaySerial)
        {
            // Controller opened successfully, so we need to start all the worker threads
            m_pollThread->Start(Driver::PollThreadEntryPoint, this);

            // Send a NAK to the ZWave device
            m_controller->Write(&nak, 1);
        }

        // Get/set ZWave controller information in its preferred initialization order
        m_controller->PlayInitSequence(this);

        //If we ever want promiscuous mode uncomment this code.
        //Msg* msg = new Msg( "FUNC_ID_ZW_SET_PROMISCUOUS_MODE", 0xff, REQUEST, FUNC_ID_ZW_SET_PROMISCUOUS_MODE, false, false );
        //msg->Append( 0xff );
        //SendMsg( msg );

        m_initMutex->Unlock();

        // Init successful
        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------

    bool Driver::InitNetworkKeys(bool _newnode)
    {
        uint8 EncryptPassword[16]       = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
        uint8 AuthPassword[16]          = {0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};
        uint8 SecuritySchemes[1][16]    = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
        uint8 tmpEncKey[32];
        uint8 tmpAuthKey[32];

        m_inclusionkeySet   = _newnode;
        m_authKey           = new aes_encrypt_ctx();
        m_encryptKey        = new aes_encrypt_ctx();

        Log::Write(LogLevel::LogLevel_Info, GetControllerNodeId(), "Setting Up %s Network Key for Secure Communications", _newnode == true ? "Inclusion" : "Provided");

        if (!IsNetworkKeySet())
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed - Network Key Not Set");

            return FALSE;
        }

        if (aes_init() == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to Init AES Engine");

            return FALSE;
        }

        if (aes_encrypt_key128(_newnode == FALSE ? this->GetNetworkKey() : SecuritySchemes[0], m_encryptKey) == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to Set Initial Network Key for Encryption");

            return FALSE;
        }

        if (aes_encrypt_key128(_newnode == FALSE ? this->GetNetworkKey() : SecuritySchemes[0], m_authKey) == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to Set Initial Network Key for Authentication");

            return FALSE;
        }

        aes_mode_reset(m_encryptKey);
        aes_mode_reset(m_authKey);

        if (aes_ecb_encrypt(EncryptPassword, tmpEncKey, 16, m_encryptKey) == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to Generate Encrypted Network Key for Encryption");

            return FALSE;
        }

        if (aes_ecb_encrypt(AuthPassword, tmpAuthKey, 16, m_authKey) == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to Generate Encrypted Network Key for Authentication");

            return FALSE;
        }

        aes_mode_reset(m_encryptKey);
        aes_mode_reset(m_authKey);

        if (aes_encrypt_key128(tmpEncKey, m_encryptKey) == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to set Encrypted Network Key for Encryption");

            return FALSE;
        }

        if (aes_encrypt_key128(tmpAuthKey, m_authKey) == EXIT_FAILURE)
        {
            Log::Write(LogLevel::LogLevel_Warning, GetControllerNodeId(), "Failed to set Encrypted Network Key for Authentication");

            return FALSE;
        }

        aes_mode_reset(m_encryptKey);
        aes_mode_reset(m_authKey);

        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::Start()
    {
        if (m_controllerInterfaceType != ControllerInterface_ZWaySerial)
        {
            // Start the thread that will handle communications with Z-Wave network
            m_driverThread->Start(Driver::DriverThreadEntryPoint, this);
        }
        else
        {
            Init(0);
        }
    }
    //----------------------------------------------------------------------------------------------------

    bool Driver::ReadMsg()
    {
        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------

    bool Driver::WriteMsg(string const& _str)
    {
        return TRUE;
    }
    //----------------------------------------------------------------------------------------------------

    bool Driver::WriteNextMsg(MsgQueue _queue)
    {
        return FALSE;
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::QueueNotification(Notification* _notification)
    {
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::NotifyWatchers()
    {
        list<Notification*>::iterator nit = m_notifications.begin();

        while (nit != m_notifications.end())
        {
            Notification* notification = m_notifications.front();

            m_notifications.pop_front();

            // Check the any ValueID's sent as part of the Notification are still valid
            switch (notification->GetType())
            {
                case NotificationType::Type_ValueChanged:
                case NotificationType::Type_ValueRefreshed:
                {
                    Value *val = GetValue(notification->GetValueID());

                    if (!val)
                    {
                        Log::Write(LogLevel::LogLevel_Info, notification->GetNodeId(), "Dropping Notification as ValueID does not exist");

                        nit = m_notifications.begin();

                        delete notification;

                        val->Release();

                        continue;
                    }

                    break;
                }
                default:
                    break;
            }

            Log::Write(LogLevel::LogLevel_Detail, notification->GetNodeId(), "Notification: %s", notification->GetAsString().c_str());

            Manager::Get()->NotifyWatchers(notification);

            delete notification;

            nit = m_notifications.begin();
        }

        m_notificationsEvent->Reset();
    }
    //----------------------------------------------------------------------------------------------------

    uint8* Driver::GetNetworkKey()
    {
        static bool keySet          = FALSE;
        static uint8 keybytes[16]   = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        unsigned int tempkey[16];
        int i;
        string networkKey;
        vector<string> elems;

        Options::Get()->GetOptionAsString("NetworkKey", &networkKey);

        if (keySet == FALSE)
        {
            OpenZWave::split(elems, networkKey, ",", TRUE);

            if (elems.size() != 16)
            {
                Log::Write(LogLevel::LogLevel_Warning, "Invalid Network Key. Does not contain 16 Bytes - Contains %d", elems.size());
                Log::Write(LogLevel::LogLevel_Warning, "Raw Key: %s", networkKey.c_str());
                Log::Write(LogLevel::LogLevel_Warning, "Parsed Key:");

                i = 0;

                for (vector<string>::iterator it = elems.begin(); it != elems.end(); it++)
                    Log::Write(LogLevel::LogLevel_Warning, "%d) - %s", ++i, (*it).c_str());

                //OZW_FATAL_ERROR(OZWException::OZWEXCEPTION_SECURITY_FAILED, "Failed to Read Network Key");
            }

            i = 0;

            for (vector<string>::iterator it = elems.begin(); it != elems.end(); it++)
            {
                if (0 == sscanf(OpenZWave::trim(*it).c_str(), "%x", &tempkey[i]))
                {
                    Log::Write(LogLevel::LogLevel_Warning, "Cannot Convert Network Key Byte %s to Key", (*it).c_str());

                    //OZW_FATAL_ERROR(OZWException::OZWEXCEPTION_SECURITY_FAILED, "Failed to Convert Network Key");
                }
                else
                {
                    keybytes[i] = (tempkey[i] & 0xFF);
                }

                i++;
            }

            keySet = true;
        }

        return keybytes;
    }
    //----------------------------------------------------------------------------------------------------

    void Driver::SendMsg(Msg* _msg, MsgQueue _queue)
    {
    }
    //----------------------------------------------------------------------------------------------------
}
