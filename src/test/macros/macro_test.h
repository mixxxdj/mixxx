#include <gtest/gtest.h>

#include "track/macro.h"

namespace {
const QString kConfigGroup = QStringLiteral("[MacroRecording]");
const QString kChannelGroup("[Channel1]");

const MacroAction kAction(25'000, 7'500);
/// Checks that a Macro containing the given action was successfully recorded,
/// including the extra first loop action.
void checkMacroAction(MacroPtr macro, MacroAction action = kAction) {
    EXPECT_EQ(macro->size(), 2);
    EXPECT_EQ(macro->getActions().last().position, action.position);
    EXPECT_EQ(macro->getActions().last().target, action.target);
    EXPECT_EQ(macro->getActions().first().position, action.target);
}
} // namespace
