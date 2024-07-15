#include "mixxxcontroller.h"

namespace mixxx {
namespace qml {
void MixxxController::init() {
    metaObject()->invokeMethod(this, "init");
}
void MixxxController::shutdown() {
    metaObject()->invokeMethod(this, "shutdown");
}
} // namespace qml
} // namespace mixxx
