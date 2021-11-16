#include "macroaction.h"

proto::Macro_Action* MacroAction::serialize() const {
    auto* serialized = new proto::Macro_Action();
    serialized->set_sourceframe(static_cast<uint64_t>(source.value()));
    serialized->set_targetframe(static_cast<uint64_t>(target.value()));
    serialized->set_type(static_cast<uint32_t>(type));
    return serialized;
}

bool operator==(const MacroAction& lhs, const MacroAction& rhs) {
    return lhs.getSourcePosition() == rhs.getSourcePosition() &&
            lhs.getTargetPosition() == rhs.getTargetPosition();
}
bool operator!=(const MacroAction& lhs, const MacroAction& rhs) {
    return !operator==(lhs, rhs);
}

QDebug operator<<(QDebug debug, const MacroAction& action) {
    debug << "Jump from" << action.getSourcePosition() << "to" << action.getTargetPosition();
    return debug;
}
