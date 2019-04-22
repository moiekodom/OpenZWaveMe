#include "../Driver.h"
#include "Controller.h"

namespace OpenZWaveMe
{
    void Controller::PlayInitSequence(Driver* _driver)
    {
        _driver->SendMsg(new Msg("FUNC_ID_ZW_GET_VERSION", 0xff, REQUEST, FUNC_ID_ZW_GET_VERSION, FALSE), MsgQueue::MsgQueue_Command);
        _driver->SendMsg(new Msg("FUNC_ID_ZW_MEMORY_GET_ID", 0xff, REQUEST, FUNC_ID_ZW_MEMORY_GET_ID, FALSE), MsgQueue::MsgQueue_Command);
        _driver->SendMsg(new Msg("FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES", 0xff, REQUEST, FUNC_ID_ZW_GET_CONTROLLER_CAPABILITIES, FALSE), MsgQueue::MsgQueue_Command);
        _driver->SendMsg(new Msg("FUNC_ID_SERIAL_API_GET_CAPABILITIES", 0xff, REQUEST, FUNC_ID_SERIAL_API_GET_CAPABILITIES, FALSE), MsgQueue::MsgQueue_Command);
        _driver->SendMsg(new Msg("FUNC_ID_ZW_GET_SUC_NODE_ID", 0xff, REQUEST, FUNC_ID_ZW_GET_SUC_NODE_ID, FALSE), MsgQueue::MsgQueue_Command);

        // FUNC_ID_ZW_GET_VIRTUAL_NODES & FUNC_ID_SERIAL_API_GET_INIT_DATA has moved into the handler for FUNC_ID_SERIAL_API_GET_CAPABILITIES
    }
    //----------------------------------------------------------------------------------------------------
}
