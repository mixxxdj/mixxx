#include <gtest/gtest.h>

#include "track/macro.h"

namespace {
const QString kConfigGroup("[MacroRecording]");
const QString kChannelGroup("[Channel1]");
} // namespace

struct TestMacro {
    MacroAction action;

    TestMacro(double sourceFramePos = 25'000, double targetFramePos = 7'500)
            : action(mixxx::audio::FramePos(sourceFramePos),
                      mixxx::audio::FramePos(targetFramePos)) {
    }

    /// Checks that the recorded Macro corresponds to the parameters,
    /// including the extra first loop action.
    void checkMacroAction(MacroPointer macro) {
        ASSERT_EQ(macro->size(), 2);
        EXPECT_EQ(macro->getActions().last().getSourcePosition().value(),
                action.getSourcePosition().value());
        EXPECT_EQ(macro->getActions().last().getTargetPosition().value(),
                action.getTargetPosition().value());
        // Loopback action
        EXPECT_EQ(macro->getActions().first().getSourcePosition().value(),
                action.getTargetPosition().value());
    }
};
