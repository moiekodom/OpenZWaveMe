#ifndef CONTROLLER_H_INCLUDED
#define CONTROLLER_H_INCLUDED

#include <platform/Stream.h>

#include <Msg.h>
#include <Driver.h>

#include "../Defs.h"

using OpenZWave::Stream;
using OpenZWave::Msg;

namespace OpenZWaveMe
{
    class Driver;

    class Controller : public Stream
    {
        using MsgQueue = OpenZWave::Driver::MsgQueue;

        ////////////////////////////////////////////////////////////////////////////////////////////////////

        public:
            Controller() : Stream(2048) {}
            virtual ~Controller() {}

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            virtual bool Open(string const& _controllerName) = 0;
            virtual bool Close() = 0;
            virtual bool IsRunning() = 0;
            virtual bool IsIdle() = 0;
            virtual uint32 Write(uint8* _buffer, uint32 _length) = 0;
            virtual void PlayInitSequence(Driver* _driver);
    };
}

#endif // CONTROLLER_H_INCLUDED
