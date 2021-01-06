#pragma once

#include "engine/engine.h"
#include "proto/macro.pb.h"
namespace proto = mixxx::track::io;

/// A MacroAction is the building block of a Macro.
/// It contains a position as well as the action to be taken at that position.
///
/// Note that currently only jumps to a target position are available, but that
/// is subject to change.
class MacroAction {
  public:
    enum class Type : uint32_t {
        Jump = 0
    };

    MacroAction(double sourceFramePos, double targetFramePos)
            : sourceFrame(sourceFramePos), targetFrame(targetFramePos), type(Type::Jump){};

    Type getType() const {
        return type;
    }

    double getSourcePosition() const {
        return sourceFrame;
    }
    double getTargetPosition() const {
        return targetFrame;
    }

    double getSourcePositionSample() const {
        return sourceFrame * mixxx::kEngineChannelCount;
    }
    double getTargetPositionSample() const {
        return targetFrame * mixxx::kEngineChannelCount;
    }

    proto::Macro_Action* serialize() const;

  private:
    // use FramePos once https://github.com/mixxxdj/mixxx/pull/2961 is merged
    double sourceFrame;
    double targetFrame;
    Type type;
};

bool operator==(const MacroAction& lhs, const MacroAction& rhs);
inline bool operator!=(const MacroAction& lhs, const MacroAction& rhs);

QDebug operator<<(QDebug debug, const MacroAction& action);
