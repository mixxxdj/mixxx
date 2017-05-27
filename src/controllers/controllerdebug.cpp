#include "controllers/controllerdebug.h"

#include "util/cmdlineargs.h"


//static
bool ControllerDebug::s_enabled = false;

//static
bool ControllerDebug::enabled() {
    return s_enabled || CmdlineArgs::Instance().getMidiDebug();
}
