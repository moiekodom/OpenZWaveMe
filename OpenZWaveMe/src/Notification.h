#ifndef NOTIFICATION_H_INCLUDED
#define NOTIFICATION_H_INCLUDED

#include <Driver.h>
#include <Notification.h>

#include "Defs.h"

#include "value_classes/ValueID.h"

using OpenZWaveMe::ValueID;

namespace OpenZWaveMe
{
    class Notification
    {
        friend class Driver;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        using ControllerState   = OpenZWave::Driver::ControllerState;
        using ControllerCommand = OpenZWave::Driver::ControllerCommand;
        using ControllerError   = OpenZWave::Driver::ControllerError;
        using NotificationType  = OpenZWave::Notification::NotificationType;
        using NotificationCode  = OpenZWave::Notification::NotificationCode;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        private:
            uint8               m_byte;
            uint8               m_event;
            NotificationType    m_type;
            ValueID             m_valueId;

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            Notification(NotificationType _type) : m_byte(0), m_event(0), m_type(_type) {}
            ~Notification() {}

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            inline uint8 GetNodeId() const;
            inline ValueID const& GetValueID() const;
            inline void SetHomeAndNodeIds(uint32 _homeId, uint8 _nodeId);
            inline NotificationType GetType() const;
            inline void SetNotification(uint8 _noteId);
            inline string GetAsString() const;
    };

    inline uint8 Notification::GetNodeId() const {return m_valueId.GetNodeId();}

    inline ValueID const& Notification::GetValueID() const {return m_valueId;}

    inline void Notification::SetHomeAndNodeIds(uint32 _homeId, uint8 _nodeId) {m_valueId = ValueID(_homeId, _nodeId);}

    inline Notification::NotificationType Notification::GetType() const {return m_type;}

    inline void Notification::SetNotification(uint8 _noteId) {ASSERT(m_type == NotificationType::Type_Notification || m_type == NotificationType::Type_ControllerCommand); m_byte = _noteId;}

    inline string Notification::GetAsString() const
    {
        string str;

        if (m_type == NotificationType::Type_ValueAdded)                            str = "ValueAdded";
        else if (m_type == NotificationType::Type_ValueRemoved)                     str = "ValueRemoved";
        else if (m_type == NotificationType::Type_ValueChanged)                     str = "ValueChanged";
        else if (m_type == NotificationType::Type_ValueRefreshed)                   str = "ValueRefreshed";
        else if (m_type == NotificationType::Type_Group)                            str = "Group";
        else if (m_type == NotificationType::Type_NodeNew)                          str = "NodeNew";
        else if (m_type == NotificationType::Type_NodeAdded)                        str = "NodeAdded";
        else if (m_type == NotificationType::Type_NodeRemoved)                      str = "NodeRemoved";
        else if (m_type == NotificationType::Type_NodeProtocolInfo)                 str = "NodeProtocolInfo";
        else if (m_type == NotificationType::Type_NodeNaming)                       str = "NodeNaming";
        else if (m_type == NotificationType::Type_NodeEvent)                        str = "NodeEvent";
        else if (m_type == NotificationType::Type_PollingDisabled)                  str = "PollingDisabled";
        else if (m_type == NotificationType::Type_PollingEnabled)                   str = "PollingEnabled";
        else if (m_type == NotificationType::Type_SceneEvent)                       str = "SceneEvent";
        else if (m_type == NotificationType::Type_CreateButton)                     str = "CreateButton";
        else if (m_type == NotificationType::Type_DeleteButton)                     str = "DeleteButton";
        else if (m_type == NotificationType::Type_ButtonOn)                         str = "ButtonOn";
        else if (m_type == NotificationType::Type_ButtonOff)                        str = "ButtonOff";
        else if (m_type == NotificationType::Type_DriverReady)                      str = "DriverReady";
        else if (m_type == NotificationType::Type_DriverFailed)                     str = "DriverFailed";
        else if (m_type == NotificationType::Type_DriverReset)                      str = "DriverReset";
        else if (m_type == NotificationType::Type_EssentialNodeQueriesComplete)     str = "EssentialNodeQueriesComplete";
        else if (m_type == NotificationType::Type_NodeQueriesComplete)              str = "NodeQueriesComplete";
        else if (m_type == NotificationType::Type_AwakeNodesQueried)                str = "AwakeNodesQueried";
        else if (m_type == NotificationType::Type_AllNodesQueriedSomeDead)          str = "AllNodesQueriedSomeDead";
        else if (m_type == NotificationType::Type_AllNodesQueried)                  str = "AllNodesQueried";
        else if (m_type == NotificationType::Type_Notification)
        {
            if (m_byte == NotificationCode::Code_MsgComplete)                       str = "Notification - MsgComplete";
            else if (m_byte == NotificationCode::Code_Timeout)                      str = "Notification - TimeOut";
            else if (m_byte == NotificationCode::Code_NoOperation)                  str = "Notification - NoOperation";
            else if (m_byte == NotificationCode::Code_Awake)                        str = "Notification - Node Awake";
            else if (m_byte == NotificationCode::Code_Sleep)                        str = "Notification - Node Asleep";
            else if (m_byte == NotificationCode::Code_Dead)                         str = "Notification - Node Dead";
            else if (m_byte == NotificationCode::Code_Alive)                        str = "Notification - Node Alive";
        }
        else if (m_type == NotificationType::Type_DriverRemoved)                    str = "DriverRemoved";
        else if (m_type == NotificationType::Type_ControllerCommand)
        {
            if (m_event == ControllerState::ControllerState_Normal)                 str = "ControllerCommand - Normal";
            else if (m_event == ControllerState::ControllerState_Starting)          str = "ControllerCommand - Starting";
            else if (m_event == ControllerState::ControllerState_Cancel)            str = "ControllerCommand - Canceled";
            else if (m_event == ControllerState::ControllerState_Error)
            {
                if (m_byte == ControllerError::ControllerError_None)                str = "ControllerCommand - Error - None";
                else if (m_byte == ControllerError::ControllerError_ButtonNotFound) str = "ControllerCommand - Error - ButtonNotFound";
                else if (m_byte == ControllerError::ControllerError_NodeNotFound)   str = "ControllerCommand - Error - NodeNotFound";
                else if (m_byte == ControllerError::ControllerError_NotBridge)      str = "ControllerCommand - Error - NotBridge";
                else if (m_byte == ControllerError::ControllerError_NotSUC)         str = "ControllerCommand - Error - NotSUC";
                else if (m_byte == ControllerError::ControllerError_NotSecondary)   str = "ControllerCommand - Error - NotSecondary";
                else if (m_byte == ControllerError::ControllerError_NotPrimary)     str = "ControllerCommand - Error - NotPrimary";
                else if (m_byte == ControllerError::ControllerError_IsPrimary)      str = "ControllerCommand - Error - IsPrimary";
                else if (m_byte == ControllerError::ControllerError_NotFound)       str = "ControllerCommand - Error - NotFound";
                else if (m_byte == ControllerError::ControllerError_Busy)           str = "ControllerCommand - Error - Busy";
                else if (m_byte == ControllerError::ControllerError_Failed)         str = "ControllerCommand - Error - Failed";
                else if (m_byte == ControllerError::ControllerError_Disabled)       str = "ControllerCommand - Error - Disabled";
                else if (m_byte == ControllerError::ControllerError_Overflow)       str = "ControllerCommand - Error - OverFlow";
            }
            else if (m_event == ControllerState::ControllerState_Waiting)           str = "ControllerCommand - Waiting";
            else if (m_event == ControllerState::ControllerState_Sleeping)          str = "ControllerCommand - Sleeping";
            else if (m_event == ControllerState::ControllerState_InProgress)        str = "ControllerCommand - InProgress";
            else if (m_event == ControllerState::ControllerState_Completed)         str = "ControllerCommand - Completed";
            else if (m_event == ControllerState::ControllerState_Failed)            str = "ControllerCommand - Failed";
            else if (m_event == ControllerState::ControllerState_NodeOK)            str = "ControllerCommand - NodeOK";
            else if (m_event == ControllerState::ControllerState_NodeFailed)        str = "ControllerCommand - NodeFailed";
        }
        else if (m_type == NotificationType::Type_NodeReset)                        str = "Node Reset";

        return str;
    }
}

#endif // NOTIFICATION_H_INCLUDED
