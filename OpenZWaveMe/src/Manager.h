#ifndef MANAGER_H_INCLUDED
#define MANAGER_H_INCLUDED

#include <platform/Mutex.h>

#include "Defs.h"

#include "Notification.h"
#include "Driver.h"

using OpenZWave::Mutex;

using OpenZWaveMe::Notification;
using OpenZWaveMe::Driver;

namespace OpenZWaveMe
{
    class Manager
    {
        friend class Driver;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        using LogLevel = OpenZWave::LogLevel;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            using ControllerInterface = Driver::ControllerInterface;

            using pfnOnNotification_t = void (*)(Notification* _pNotification, void* _context);

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        private:

            struct Watcher
            {
                pfnOnNotification_t	        m_callback;
                void*				        m_context;

                Watcher(pfnOnNotification_t _callback, void* _context) : m_callback(_callback), m_context(_context) {}
            };

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            static Manager*                 s_instance;

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            Mutex*							m_notificationMutex;

            list<Watcher*>		            m_watchers;
            list<list<Watcher*>::iterator*> m_watcherIterators;
            list<Driver*>                   m_pendingDrivers;
            map<uint32, Driver*>            m_readyDrivers;

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            Manager();
            virtual ~Manager();

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            void SetDriverReady(Driver* _driver, bool _success);
            void NotifyWatchers(Notification* _notification);

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            static Manager* Create();
            static void Destroy();
            static Manager* Get() {return s_instance;}

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            bool AddDriver(string const& _controllerPath, ControllerInterface _interface = ControllerInterface::ControllerInterface_ZWaySerial);
            bool RemoveDriver(string const& _controllerPath);
            bool AddWatcher(pfnOnNotification_t _watcher, void* _context);
    };
}

#endif // MANAGER_H_INCLUDED
