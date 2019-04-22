#ifndef VALUEID_H_INCLUDED
#define VALUEID_H_INCLUDED

#include <value_classes/ValueID.h>

#include "../Defs.h"

namespace OpenZWaveMe
{
    class ValueID
    {
        friend class Driver;
        friend class Node;
        friend class Notification;

        private:
            uint32 m_id;
            uint32 m_id1;
            uint32 m_homeId;

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            ValueID() :
                m_id(0),
                m_id1(0),
                m_homeId(0) {}
            ValueID(uint32 _homeId, uint8 _nodeId) :
                m_id1(0),
                m_homeId(_homeId) {m_id = ((uint32)_nodeId) << 24;}

            inline uint32 GetValueStoreKey() const;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            inline uint8 GetNodeId() const;
            inline uint8 GetCommandClassId() const;
            inline uint8 GetInstance()const;
            inline uint8 GetIndex() const;
    };

    inline uint32 ValueID::GetValueStoreKey() const {return (m_id & 0x003ffff0) | (m_id1 & 0xff000000);}

    inline uint8 ValueID::GetNodeId() const {return (uint8)((m_id & 0xff000000) >> 24);}

    inline uint8 ValueID::GetCommandClassId() const {return (uint8)((m_id & 0x003fc000) >> 14);}

    inline uint8 ValueID::GetInstance() const {return (uint8)((m_id1 & 0xff000000) >> 24);}

    inline uint8 ValueID::GetIndex() const {return (uint8)((m_id & 0x00000ff0) >> 4);}
}

#endif // VALUEID_H_INCLUDED
