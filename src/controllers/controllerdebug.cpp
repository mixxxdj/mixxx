#include "controllers/controllerdebug.h"

#include "util/cmdlineargs.h"


//static
bool ControllerDebug::s_enabled = false;
bool ControllerDebug::s_testing = false;

//static
bool ControllerDebug::isEnabled() {
    return s_enabled || CmdlineArgs::Instance().getMidiDebug();
}
