#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED

#include <platform/TimeStamp.h>
#include <platform/Event.h>
#include <platform/Mutex.h>
#include <platform/Thread.h>
#include <platform/Wait.h>

#include <aes/aescpp.h>

#include <value_classes/Value.h>

#include <command_classes/WakeUp.h>

#include <Msg.h>
#include <Driver.h>

#include "Defs.h"

#include "platform/ZWaySerialController.h"

#include "Notification.h"
#include "Node.h"

using OpenZWave::TimeStamp;
using OpenZWave::Event;
using OpenZWave::Mutex;
using OpenZWave::Thread;
using OpenZWave::Wait;
using OpenZWave::Value;
using OpenZWave::WakeUp;
using OpenZWave::Msg;
using OpenZWave::LockGuard;

using OpenZWaveMe::Controller;
using OpenZWaveMe::Notification;
using OpenZWaveMe::Node;

namespace OpenZWaveMe
{
    class Driver
    {
        friend class Manager;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        using LogLevel          = OpenZWave::LogLevel;
        using ControllerState   = OpenZWave::Driver::ControllerState;
        using ControllerCommand = OpenZWave::Driver::ControllerCommand;
        using ControllerError   = OpenZWave::Driver::ControllerError;
        using MsgQueue          = OpenZWave::Driver::MsgQueue;
        using NotificationType  = OpenZWave::Notification::NotificationType;
        using NotificationCode  = OpenZWave::Notification::NotificationCode;

        using pfnControllerCallback_t = void (*)(ControllerState _state, ControllerError _err, void* context);

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            enum ControllerInterface
            {
                ControllerInterface_Unknown = 0,
                ControllerInterface_Serial,
                ControllerInterface_ZWaySerial,
                ControllerInterface_Hid
            };

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        private:
            enum MsgQueueCmd
            {
                MsgQueueCmd_SendMsg = 0,
                MsgQueueCmd_QueryStageComplete,
                MsgQueueCmd_Controller
            };

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            struct ControllerCommandItem
            {
                ControllerState		    m_controllerState;
                bool					m_controllerStateChanged;
                bool					m_controllerCommandDone;
                ControllerCommand		m_controllerCommand;
                pfnControllerCallback_t m_controllerCallback;
                ControllerError			m_controllerReturnError;
                void*					m_controllerCallbackContext;
                bool					m_highPower;
                bool					m_controllerAdded;
                uint8					m_controllerCommandNode;
                uint8					m_controllerCommandArg;
                uint8					m_controllerDeviceProtocolInfo[254];
                uint8 					m_controllerDeviceProtocolInfoLength;
            };

            struct PollEntry
            {
                ValueID	                m_id;
                uint8	                m_pollCounter;
            };

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            class MsgQueueItem
            {
            public:
                bool				    m_retry;
                uint8				    m_nodeId;
                Node::QueryStage        m_queryStage;
                MsgQueueCmd			    m_command;
                Msg*				    m_msg;
                ControllerCommandItem*  m_cci;

                ////////////////////////////////////////////////////////////////////////////////////////////////////

                MsgQueueItem() :
                    m_msg(NULL),
                    m_nodeId(0),
                    m_queryStage(Node::QueryStage_None),
                    m_retry(FALSE),
                    m_cci(NULL) {}

                ////////////////////////////////////////////////////////////////////////////////////////////////////

                bool operator == (MsgQueueItem const& _other) const
                {
                    if (_other.m_command == m_command)
                    {
                        if (m_command == MsgQueueCmd_SendMsg)
                        {
                            return *_other.m_msg == *m_msg;
                        }
                        else if (m_command == MsgQueueCmd_QueryStageComplete)
                        {
                            return (_other.m_nodeId == m_nodeId) && (_other.m_queryStage == m_queryStage);
                        }
                        else if (m_command == MsgQueueCmd_Controller)
                        {
                            return (_other.m_cci->m_controllerCommand == m_cci->m_controllerCommand) && (_other.m_cci->m_controllerCallback == m_cci->m_controllerCallback);
                        }
                    }

                    return FALSE;
                }
            };

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            bool					m_bIntervalBetweenPolls;					        // If true, the library intersperses m_pollInterval between polls; if false, the library attempts to complete all polls within m_pollInterval
            bool                    m_waitingForAck;                                    // True when we are waiting for an ACK from the dongle
            bool                    m_awakeNodesQueried;                                // Set to true once the driver has polled all awake nodes
            bool					m_notifytransactions;
            bool                    m_exit;                                             // Flag that is set when the application is exiting
            bool                    m_inclusionkeySet;
            uint8                   m_Controller_nodeId;                                // Z-Wave Controller's own node ID
            uint8                   m_expectedCallbackId;                               // If non-zero, we wait for a message with this callback Id
            uint8                   m_expectedReply;                                    // If non-zero, we wait for a message with this function Id
            uint8		            m_virtualNeighbors[NUM_NODE_BITFIELD_BYTES];		// Bitmask containing virtual neighbors
            uint32                  m_homeId;                                           // Home ID of the Z-Wave controller. Not valid until the DriverReady notification has been received.
            int32                   m_pollInterval;                                     // Time interval during which all nodes must be polled
            string                  m_controllerPath;                                   // Name or path used to open the controller hardware
            ControllerInterface     m_controllerInterfaceType;                          // Specifies the controller's hardware interface
            ControllerCommandItem*  m_currentControllerCommand;
            Controller*             m_controller;                                       // Handles communications with the controller hardware
            aes_encrypt_ctx*        m_authKey;
            aes_encrypt_ctx*        m_encryptKey;
            Mutex*                  m_initMutex;                                        // Mutex to ensure proper ordering of initialization/deinitialization
            Mutex*                  m_pollMutex;                                        // Serialize access to the polling list
            Mutex*                  m_nodeMutex;								        // Serializes access to node data
            Thread*                 m_driverThread;                                     // Thread for reading from the Z-Wave controller, and for creating and managing the other threads for sending, polling etc.
            Thread*                 m_pollThread;                                       // Thread for polling devices on the Z-Wave network
            Event*                  m_notificationsEvent;
            Event*                  m_queueEvent[OpenZWave::Driver::MsgQueue_Count];    // Events for each queue, which are signaled when the queue is not empty
            Msg*                    m_currentMsg;
            Node*                   m_nodes[256];                                       // Array containing all the node objects

            list<Notification*>     m_notifications;
            list<PollEntry>         m_pollList;
            list<MsgQueueItem>		m_msgQueue[OpenZWave::Driver::MsgQueue_Count];      // List of nodes that need to be polled

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            Driver(string const& _controllerPath, ControllerInterface const& _interface);
            virtual ~Driver();

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            static void DriverThreadEntryPoint(Event* _exitEvent, void* _context);
            static void PollThreadEntryPoint(Event* _exitEvent, void* _context);

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            void DriverThreadProc(Event* _exitEvent);
            void PollThreadProc(Event* _exitEvent);
            bool Init(uint32 _attempts);
            bool InitNetworkKeys(bool _newnode);
            void Start();
            bool ReadMsg();
            bool WriteMsg(string const& _str);
            bool WriteNextMsg(MsgQueue _queue);
            void QueueNotification(Notification* _notification);
            void NotifyWatchers();
            inline string const& GetControllerPath();
            inline uint8 GetControllerNodeId();
            inline Node* GetNode(uint8 _nodeId);
            inline Value* GetValue(ValueID const& _id);
            uint8* GetNetworkKey();

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            void SendMsg(Msg* _msg, MsgQueue _queue);
            inline bool IsNetworkKeySet();
    };

    inline string const& Driver::GetControllerPath() {return m_controllerPath;}

    inline uint8 Driver::GetControllerNodeId() {return m_Controller_nodeId;}

    inline Node* Driver::GetNode(uint8 _nodeId)
    {
        Node* node = m_nodes[_nodeId];

        if (m_nodeMutex->IsSignalled())
        {
            Log::Write(LogLevel::LogLevel_Error, _nodeId, "Driver Thread is Not Locked during Call to GetNode");

            return NULL;
        }

        if (node) return node;

        return NULL;
    }

    inline Value* Driver::GetValue(ValueID const& _id) {Node* node = m_nodes[_id.GetNodeId()]; return node ? node->GetValueOverridden(_id ) : NULL;}

    inline bool Driver::IsNetworkKeySet()
    {
        string networkKey;

        if (!Options::Get()->GetOptionAsString("NetworkKey", &networkKey)) return FALSE;
        else return networkKey.length() <= 0 ? FALSE : TRUE;
    }
}

#endif // DRIVER_H_INCLUDED
