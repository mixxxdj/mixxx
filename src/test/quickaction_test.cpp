#include "quickaction_test.h"

MacroRecorderTest::MacroRecorderTest()
        : pMacroRecorder(new ThreadLocalQuickAction()),
          co1(ConfigKey("[Test]", "control1")),
          co2(ConfigKey("[Test]", "control2")),
          coRecording("[QuickAction]", "recording"),
          coTrigger("[QuickAction]", "trigger") {
    co1.setMacroRecorder(pMacroRecorder);
    co2.setMacroRecorder(pMacroRecorder);
    co1.setMacroRecordable(true);
    co2.setMacroRecordable(true);

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

TEST_F(MacroRecorderTest, ValuesAreSetInRecordingOrder) {
    coRecording.set(1);
    co1.set(1);
    co2.set(2);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    EXPECT_EQ(co2.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(co2.get(), 2) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(result.m_value, 2) << "Values are not set in the order they were recorded";
    EXPECT_EQ(setCount.m_value, 2) << "Value set too many or too few times";

    // Reset values for next case
    co1.set(0);
    co2.set(0);
    setCount.m_value = 0;

    // Same test setting control2 first, to make sure ConfigKey order does not interfere with the triggering order.
    coRecording.set(1);
    co2.set(2);
    co1.set(1);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    EXPECT_EQ(co2.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(co2.get(), 2) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(result.m_value, 1) << "Values are not set in the order they were recorded";
    EXPECT_EQ(setCount.m_value, 2) << "Value set too many or too few times";
}

TEST_F(MacroRecorderTest, OldValuesAreOverwritten) {
    coRecording.set(1);
    co1.set(1);
    co1.set(2);
    coRecording.set(0);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 2) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(setCount.m_value, 1)
            << "New Recorded value was appended to the recording instead of "
               "taking the place of the old value";
}

TEST_F(MacroRecorderTest, TriggerDisablesRecording) {
    coRecording.set(1);
    co1.set(1);
    EXPECT_EQ(co1.get(), 0) << "Values are set while recording";
    coTrigger.set(1);
    EXPECT_EQ(co1.get(), 1) << "Recorded values are not set when macro is triggered";
    EXPECT_EQ(coRecording.get(), 0) << "QuickAction is still recording";
}

TEST_F(MacroRecorderTest, NonRecordableValuesAreNotPerturbed) {
    co1.setMacroRecordable(false);
    coRecording.set(1);
    co1.set(1);
    EXPECT_EQ(co1.get(), 1) << "NonRecordable value is filtered while recording";
    coTrigger.set(1);
    EXPECT_EQ(setCount.m_value, 1) << "NonRecordable value is set again when "
                                      "triggering the QuickAction";
}
