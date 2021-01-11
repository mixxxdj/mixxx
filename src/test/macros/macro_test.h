#include <gtest/gtest.h>

#include "track/macro.h"

namespace {
const QString kConfigGroup("[MacroRecording]");
const QString kChannelGroup("[Channel1]");
} // namespace

struct TestMacro {
    MacroAction action;

    TestMacro(double sourceFramePos = 25'000, double targetFramePos = 7'500)
            : action(sourceFramePos, targetFramePos) {
    }

    /// Checks that the recorded Macro corresponds to the parameters,
    /// including the extra first loop action.
    void checkMacroAction(MacroPointer macro) {
        ASSERT_EQ(macro->size(), 2);
        EXPECT_EQ(macro->getActions().last().getSourcePosition(), action.getSourcePosition());
        EXPECT_EQ(macro->getActions().last().getTargetPosition(), action.getTargetPosition());
        // Loopback action
        EXPECT_EQ(macro->getActions().first().getSourcePosition(), action.getTargetPosition());
    }
};
