#ifndef ZWAYSERIALCONTROLLER_H_INCLUDED
#define ZWAYSERIALCONTROLLER_H_INCLUDED

// Необходимо прежде включить typedef для __socklen_t, иначе при компиляции под Windows (MinGW) ошибка: "Тип не определён".
#include "../Defs.h"

// Здесь включается <bits/socket.h>, в котором есть "typedef __socklen_t socklen_t".
#include <ZWayLib.h>

#include "Controller.h"

using OpenZWaveMe::Controller;

namespace OpenZWaveMe
{
    class ZWaySerialController : public Controller
    {
        using LogLevel = OpenZWave::LogLevel;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        private:
            ZWError m_lastError;
            ZWay m_zway;

        public:
            ZWaySerialController();
            virtual ~ZWaySerialController();

            bool Open(string const& _serialControllerName);
            bool Close();
            bool IsRunning();
            bool IsIdle();
            uint32 Write(uint8* _buffer, uint32 _length);
            void PlayInitSequence(Driver* _driver);
    };
}

#endif // ZWAYSERIALCONTROLLER_H_INCLUDED
