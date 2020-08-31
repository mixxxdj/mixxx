#include <gtest/gtest.h>

#include "track/macro.h"

const QString kConfigGroup("[MacroRecording]");
const QString kChannelGroup("[Channel1]");

const MacroAction kAction(25'000, 7'500);

/// Checks that a Macro containing the given action was successfully recorded,
/// including the extra first loop action.
void checkMacroAction(MacroPointer macro, MacroAction action = kAction) {
    ASSERT_EQ(macro->size(), 2);
    EXPECT_EQ(macro->getActions().last().sourceFrame, action.sourceFrame);
    EXPECT_EQ(macro->getActions().last().targetFrame, action.targetFrame);
    // Loopback action
    EXPECT_EQ(macro->getActions().first().sourceFrame, action.targetFrame);
}
