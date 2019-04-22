#ifndef NODE_H_INCLUDED
#define NODE_H_INCLUDED

#include <tinyxml.h>

#include <platform/TimeStamp.h>

#include <value_classes/Value.h>
#include <value_classes/ValueStore.h>

#include <command_classes/CommandClass.h>

#include <Node.h>
#include <Group.h>

#include "Defs.h"

#include "value_classes/ValueID.h"

using OpenZWave::TimeStamp;
using OpenZWave::Value;
using OpenZWave::ValueStore;
using OpenZWave::CommandClass;
using OpenZWave::Group;

using OpenZWaveMe::ValueID;

namespace OpenZWaveMe
{
    class Node : public OpenZWave::Node
    {
        friend class Driver;

        private:

            // Container for device class info
			class DeviceClass
			{
			    private:
                    uint8*  m_mandatoryCommandClasses;  // Zero terminated array of mandatory command classes for this device type.
                    uint8   m_basicMapping;				// Command class that COMMAND_CLASS_BASIC maps on to, or zero if there is no mapping.
                    string  m_label;				    // Descriptive label for the device.

                ////////////////////////////////////////////////////////////////////////////////////////////////////

				public:
                    DeviceClass(TiXmlElement const* _el);
                    ~DeviceClass() {delete [] m_mandatoryCommandClasses;}

                    ////////////////////////////////////////////////////////////////////////////////////////////////////

                    uint8 const* GetMandatoryCommandClasses() {return m_mandatoryCommandClasses;}
                    uint8 GetBasicMapping() {return m_basicMapping;}
                    string const& GetLabel() {return m_label;}
			};

            // Container for generic device class info
			class GenericDeviceClass : public DeviceClass
			{
				private:
					map<uint8, DeviceClass*> m_specificDeviceClasses;

				////////////////////////////////////////////////////////////////////////////////////////////////////

                public:
					GenericDeviceClass(TiXmlElement const* _el);
					~GenericDeviceClass();

					DeviceClass* GetSpecificDeviceClass(uint8 const& _specific);
			};

			////////////////////////////////////////////////////////////////////////////////////////////////////

            QueryStage	                            m_queryStage;
			bool		                            m_queryPending;
			bool		                            m_queryConfiguration;
			uint8		                            m_queryRetries;
			bool		                            m_protocolInfoReceived;
			bool		                            m_basicprotocolInfoReceived;
			bool		                            m_nodeInfoReceived;
			bool		                            m_nodePlusInfoReceived;
			bool		                            m_manufacturerSpecificClassReceived;
			bool		                            m_nodeInfoSupported;
			bool		                            m_refreshonNodeInfoFrame;
			bool		                            m_nodeAlive;
			bool		                            m_listening;
			bool		                            m_frequentListening;
			bool		                            m_beaming;
			bool		                            m_routing;
			uint32		                            m_maxBaudRate;
			uint8		                            m_version;
			bool		                            m_security;
			uint32		                            m_homeId;
			uint8		                            m_nodeId;
			uint8		                            m_basic;		            // Basic device class (0x01-Controller, 0x02-Static Controller, 0x03-Slave, 0x04-Routing Slave
			uint8		                            m_generic;
			uint8		                            m_specific;
			string		                            m_type;				        // Label representing the specific/generic/basic value
			uint8		                            m_neighbors[29];	        // Bitmask containing the neighboring nodes
			uint8		                            m_numRouteNodes;	        // number of node routes
			uint8		                            m_routeNodes[5];	        // nodes to route to
			map<uint8, uint8>                       m_buttonMap;	            // Map button IDs into virtual node numbers
			bool		                            m_addingNode;
			string		                            m_manufacturerName;
			string		                            m_productName;
			string		                            m_nodeName;
			string		                            m_location;
			uint16		                            m_manufacturerId;
			uint16		                            m_productType;
			uint16		                            m_productId;
			map<uint8, CommandClass*>               m_commandClassMap;	        // Map of command class ids and pointers to associated command class objects */
			bool                                    m_secured;                  // Is this Node added Securely
			ValueStore*	m_values;			                                    // Values reported via command classes
			map<uint8, Group*> m_groups;					                    // Maps group indices to Group objects.
			static bool								s_deviceClassesLoaded;		// True if the xml file has already been loaded
			static map<uint8, string>				s_basicDeviceClasses;		// Map of basic device classes.
			static map<uint8, GenericDeviceClass*>	s_genericDeviceClasses;		// Map of generic device classes.
			static map<uint8, DeviceClass*> 		s_roleDeviceClasses;		// Map of Zwave+ role device classes.
			static map<uint16, DeviceClass*> 		s_deviceTypeClasses;		// Map of Zwave+ device type device classes.
			static map<uint8, DeviceClass*>			s_nodeTypes;				// Map of ZWave+ Node Types
			uint32 m_sentCnt;				                                    // Number of messages sent from this node.
			uint32 m_sentFailed;				                                // Number of sent messages failed
			uint32 m_retries;				                                    // Number of message retries
			uint32 m_receivedCnt;				                                // Number of messages received from this node.
			uint32 m_receivedDups;				                                // Number of duplicated messages received;
			uint32 m_receivedUnsolicited;			                            // Number of messages received unsolicited
			uint32 m_lastRequestRTT;			                                // Last message request RTT
			uint32 m_lastResponseRTT;			                                // Last message response RTT
			TimeStamp m_sentTS;				                                    // Last message sent time
			TimeStamp m_receivedTS;				                                // Last message received time
			uint32 m_averageRequestRTT;			                                // Average Request round trip time.
			uint32 m_averageResponseRTT;			                            // Average Response round trip time.
			uint8 m_quality;				                                    // Node quality measure
			uint8 m_lastReceivedMessage[254];		                            // Place to hold last received message
			uint8 m_errors;					                                    // Count errors for dead node detection
			uint8 m_lastnonce;
			uint8 m_nonces[8][8];

			////////////////////////////////////////////////////////////////////////////////////////////////////

			inline ValueStore* GetValueStore() const;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            Node(uint32 _homeId, uint8 _nodeId) : OpenZWave::Node(_homeId, _nodeId) {}

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            inline static Node* Clone(OpenZWave::Node* _node);

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            inline Value* GetValueOverridden(ValueID const& _id);
    };

    inline ValueStore* Node::GetValueStore() const {return m_values;}

    inline Node* Node::Clone(OpenZWave::Node* _node) {return static_cast<Node*>(_node);}

    inline Value* Node::GetValueOverridden(ValueID const& _id) {return GetValueStore()->GetValue(_id.GetValueStoreKey());}
}

#endif // NODE_H_INCLUDED
