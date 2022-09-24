#include "quickaction_test.h"

QuickActionTest::QuickActionTest()
        : pQuickActionsManager(new QuickActionsManager()),
          co1(ConfigKey("[Test]", "control1")),
          co2(ConfigKey("[Test]", "control2")),
          cp1(ConfigKey("[Test]", "control1")),
          cp2(ConfigKey("[Test]", "control2")),
          coRecording("[QuickAction1]", "recording"),
          coTrigger("[QuickAction1]", "trigger"),
          coClear("[QuickAction1]", "clear") {
    QuickActionsManager::setGlobalInstance(pQuickActionsManager);
    co1.setQuickActionsRecordable(true);
    co2.setQuickActionsRecordable(true);
    cp1.setValueChangesAreQuickActionsRecordable(true);
    cp2.setValueChangesAreQuickActionsRecordable(true);

    QObject::connect(&co1,
            &ControlObject::valueChanged,
            &result,
            &Value::slotSetValue,
            Qt::DirectConnection);
    QObject::connect(&co2,
            &ControlObject::valueChanged,
            &result,
            &Value::slotSetValue,
            Qt::DirectConnection);
    QObject::connect(&co1,
            &ControlObject::valueChanged,
            &setCount,
            &Counter::slotSetValue,
            Qt::DirectConnection);
    QObject::connect(&co2,
            &ControlObject::valueChanged,
            &setCount,
            &Counter::slotSetValue,
            Qt::DirectConnection);
}

QuickActionTest::~QuickActionTest() {
    QuickActionsManager::setGlobalInstance(nullptr);
}

TEST_F(QuickActionTest, ValuesAreSetInRecordingOrder) {
    coRecording.set(1);
    cp1.set(1);
    cp2.set(2);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "While recording, values are expected to be recorded.";
    EXPECT_EQ(co2.get(), 0) << "While recording, values are expected to be recorded.";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are expected not to be set "
                               "after the macro is triggered";
    EXPECT_EQ(co2.get(), 2) << "Recorded values are expected not to be set "
                               "after the macro is triggered";
    EXPECT_EQ(result.m_value, 2) << "Values are expected to be set in the order they were recorded";
    EXPECT_EQ(setCount.m_value, 2) << "Each value is expected to be set exactly once";

    // Reset values for next case
    cp1.set(0);
    cp2.set(0);
    setCount.m_value = 0;

    // Same test setting control2 first, to make sure ConfigKey order does not interfere with the triggering order.
    coRecording.set(1);
    cp2.set(2);
    cp1.set(1);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "While recording, values are expected to be recorded.";
    EXPECT_EQ(co2.get(), 0) << "While recording, values are expected to be recorded.";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are expected not to be set "
                               "after the macro is triggered";
    EXPECT_EQ(co2.get(), 2) << "Recorded values are expected not to be set "
                               "after the macro is triggered";
    EXPECT_EQ(result.m_value, 1) << "Values are expected to be set in the order they were recorded";
    EXPECT_EQ(setCount.m_value, 2) << "Each value is expected to be set exactly once";
}

TEST_F(QuickActionTest, OldValuesAreOverwritten) {
    coRecording.set(1);
    cp1.set(1);
    cp1.set(2);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 2) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(setCount.m_value, 1)
            << "New Recorded value was appended to the recording instead of "
               "taking the place of the old value";
}

TEST_F(QuickActionTest, TriggerDisablesRecording) {
    coRecording.set(1);
    cp1.set(1);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(coRecording.get(), 0) << "QuickAction is still recording";
}

TEST_F(QuickActionTest, NonRecordableValuesAreNotPerturbed) {
    co1.setQuickActionsRecordable(false);
    coRecording.set(1);
    cp1.set(1);
    EXPECT_EQ(co1.get(), 1) << "NonRecordable value is filtered while recording";
    EXPECT_EQ(setCount.m_value, 1) << "NonRecordable value is filtered while recording";
    coTrigger.set(1);
    EXPECT_EQ(setCount.m_value, 1) << "NonRecordable value is set when "
                                      "triggering the QuickAction";
}

TEST_F(QuickActionTest, NoValuesAreTriggeredAfterClear) {
    coRecording.set(1);
    cp1.set(1);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    coClear.set(1);
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 0) << "Recorded values are not cleared";

    coRecording.set(1);
    cp1.set(1);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are not recorded after a clear";
}
