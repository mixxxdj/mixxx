#include <gtest/gtest.h>

#include "macros/macrorecorder.h"

namespace {
const QString kConfigGroup = QStringLiteral("[MacroRecording]");

const MacroAction s_action(500, 15);
void checkRecordedAction(MacroRecorder* recorder, MacroAction action = s_action) {
    EXPECT_EQ(recorder->getRecordingSize(), 1);
    auto recordedAction = recorder->fetchRecordedActions().first();
    EXPECT_EQ(recordedAction.position, action.position);
    EXPECT_EQ(recordedAction.target, action.target);
}
} // namespace
