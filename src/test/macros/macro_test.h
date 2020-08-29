#include <gtest/gtest.h>

#include "track/macro.h"

namespace {
const QString kConfigGroup = QStringLiteral("[MacroRecording]");
const QString kChannelGroup("[Channel1]");

const MacroAction kAction(25'000, 7'500);
inline void checkMacroAction(MacroPtr macro, MacroAction action = kAction) {
    EXPECT_EQ(macro->size(), 2);
    EXPECT_EQ(macro->getActions().last(), action);
    EXPECT_EQ(macro->getActions().first().position, action.target);
}
} // namespace
