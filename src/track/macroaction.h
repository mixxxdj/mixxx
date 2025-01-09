#pragma once

#include "audio/frame.h"
#include "engine/engine.h"
#include "proto/macro.pb.h"
namespace proto = mixxx::track::io;

/// A MacroAction is the building block of a Macro.
/// It contains a position as well as the action to be taken at that position.
///
/// Note that currently only jumps to a m_target position are available,
/// but that is subject to change.
class MacroAction {
  public:
    enum class Type : uint32_t {
        Jump = 0
    };

    MacroAction(mixxx::audio::FramePos source, mixxx::audio::FramePos target)
            : m_source(source),
              m_target(target),
              m_type(Type::Jump) {
    }

    Type getType() const {
        return m_type;
    }

    mixxx::audio::FramePos getSourcePosition() const {
        return m_source;
    }
    mixxx::audio::FramePos getTargetPosition() const {
        return m_target;
    }

    proto::Macro_Action serialize() const;

  private:
    mixxx::audio::FramePos m_source;
    mixxx::audio::FramePos m_target;
    Type m_type;
};

bool operator==(const MacroAction& lhs, const MacroAction& rhs);
inline bool operator!=(const MacroAction& lhs, const MacroAction& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug debug, const MacroAction& action);
