#ifndef OZWEXCEPTION_H_INCLUDED
#define OZWEXCEPTION_H_INCLUDED

// Необходимо прежде включить typedef для __socklen_t, иначе при компиляции под Windows (MinGW) ошибка: "тип не определён".
#include "Defs.h"

// Здесь включается <bits/socket.h>, в котором есть "typedef __socklen_t socklen_t".
#include <ZErrors.h>

namespace OpenZWaveMe
{
    class OZWException : public exception
    {
        public:
            enum ZWayExceptionType
            {
                ZWAYEXCEPTION_PACKET_TOO_BIG    = PacketTooBig,
                ZWAYEXCEPTION_DUPLICATE_OBJECT  = DuplicateObject,
                ZWAYEXCEPTION_JOB_ALREADY_ADDED,
                ZWAYEXCEPTION_JOB_NOT_FOUND,
                ZWAYEXCEPTION_SOFT_FAILURE      = SoftFailure,
                ZWAYEXCEPTION_NOT_PRIMARY,
                ZWAYEXCEPTION_INVALID_CONFIG    = InvalidConfig,
                ZWAYEXCEPTION_INVALID_PORT,
                ZWAYEXCEPTION_INVALID_THREAD    = InvalidThread,
                ZWAYEXCEPTION_INVALID_TYPE      = InvalidType,
                ZWAYEXCEPTION_BAD_DATA,
                ZWAYEXCEPTION_INTERNAL_ERROR,
                ZWAYEXCEPTION_INVALID_OPERATION,
                ZWAYEXCEPTION_THREADING_ERROR,
                ZWAYEXCEPTION_ACCESS_DENIED,
                ZWAYEXCEPTION_NOT_SUPPORTED,
                ZWAYEXCEPTION_NOT_IMPLEMENTED,
                ZWAYEXCEPTION_BAD_ALLOCATION,
                ZWAYEXCEPTION_INVALID_ARG
            };

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            /*OZWException(string const& _file, int _line, ExceptionType _exitCode, string const& _msg) :
                OpenZWave::OZWException(_file, _line, _exitCode, _msg) {}*/
            OZWException();
            virtual ~OZWException() throw() {}

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            inline static bool IsZWayError(int8 _exitCode);

            ////////////////////////////////////////////////////////////////////////////////////////////////////

            virtual const char* what() const throw();
    };

    inline bool OZWException::IsZWayError(int8 _exitCode)
    {
        return
            _exitCode == ZWAYEXCEPTION_INVALID_ARG ||
            _exitCode == ZWAYEXCEPTION_BAD_ALLOCATION ||
            _exitCode == ZWAYEXCEPTION_NOT_IMPLEMENTED ||
            _exitCode == ZWAYEXCEPTION_NOT_SUPPORTED ||
            _exitCode == ZWAYEXCEPTION_ACCESS_DENIED ||
            _exitCode == ZWAYEXCEPTION_THREADING_ERROR ||
            _exitCode == ZWAYEXCEPTION_INVALID_OPERATION ||
            _exitCode == ZWAYEXCEPTION_INTERNAL_ERROR ||
            _exitCode == ZWAYEXCEPTION_BAD_DATA ||
            _exitCode == ZWAYEXCEPTION_INVALID_TYPE ||
            _exitCode == ZWAYEXCEPTION_INVALID_THREAD ||
            _exitCode == ZWAYEXCEPTION_INVALID_PORT ||
            _exitCode == ZWAYEXCEPTION_INVALID_CONFIG ||
            _exitCode == ZWAYEXCEPTION_NOT_PRIMARY ||
            _exitCode == ZWAYEXCEPTION_SOFT_FAILURE ||
            _exitCode == ZWAYEXCEPTION_JOB_NOT_FOUND ||
            _exitCode == ZWAYEXCEPTION_JOB_ALREADY_ADDED ||
            _exitCode == ZWAYEXCEPTION_DUPLICATE_OBJECT ||
            _exitCode == ZWAYEXCEPTION_PACKET_TOO_BIG;
    }
}

#endif // OZWEXCEPTION_H_INCLUDED
