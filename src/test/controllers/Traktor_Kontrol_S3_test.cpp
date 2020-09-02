#include <QThread>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/controllerdebug.h"
#include "controllers/controllerengine.h"
#include "controllers/softtakeover.h"
#include "effects/effectchain.h"
#include "effects/effectsmanager.h"
#include "test/controllers/controllertest.h"
#include "test/signalpathtest.h"

class TraktorS3Test : public ControllerTest {
  protected:
    void SetUp() override {
        ControllerTest::SetUp();

        const QString commonScript = "./res/controllers/common-controller-scripts.js";
        const QString hidScript = "./res/controllers/common-hid-packet-parser.js";
        const QString scriptFile = "./res/controllers/Traktor-Kontrol-S3-hid-scripts.js";
        ASSERT_TRUE(m_pCEngine->evaluate(commonScript));
        ASSERT_FALSE(m_pCEngine->hasErrors(commonScript));
        ASSERT_TRUE(m_pCEngine->evaluate(hidScript));
        ASSERT_FALSE(m_pCEngine->hasErrors(hidScript));
        ASSERT_TRUE(m_pCEngine->evaluate(scriptFile));
        ASSERT_FALSE(m_pCEngine->hasErrors(scriptFile));

        // Create useful objects and getters
        evaluate(
                "var TestOb = {};"
                "TestOb.fxc = new TraktorS3.FXControl(); "
                "var getState = function() {"
                " return TestOb.fxc.currentState;"
                "};"
                "var getActiveFx = function() {"
                " return TestOb.fxc.activeFX;"
                "};"
                "var getSelectPressed = function() {"
                " return TestOb.fxc.selectPressed;"
                "};"
                "var getEnablePressed = function() {"
                " return TestOb.fxc.enablePressed;"
                "};"
                // Mock out shift key.
                "TestOb.shiftPressed = false;"
                "TraktorS3.anyShiftPressed = function() {"
                "  return TestOb.shiftPressed;"
                "}");

        // Mock out controller for testing lights
        evaluate(
                "TraktorS3.FXControl.prototype.getFXSelectLEDValue = function(fxNumber, enabled) {"
                "  return fxNumber*10 + (enabled ? 6 : 5);"
                "};"
                "TraktorS3.FXControl.prototype.getChannelColor = function(group, enabled) {"
                "  return parseInt(group[8])*10 + (enabled ? 1 : 0);"
                "};"
                "TestOb.fxc.controller = new function() {"
                "  this.lightMap = {}; "
                "  this.setOutput = function(group, key, value, batching) {"
                "    if (!(group in this.lightMap)) {"
                "      this.lightMap[group] = {};"
                "    }"
                "    HIDDebug('light: ' + group + ' ' + key + ' ' + value);"
                "    this.lightMap[group][key] = value;"
                "  };"
                "};"
                "var getLight = function(group, key) {"
                "  if (!(group in TestOb.fxc.controller.lightMap)) {"
                "    return undefined;"
                "  }"
                "  return TestOb.fxc.controller.lightMap[group][key];"
                "};");
    }

    void CheckSelectLights(const std::vector<int>& expected) {
        EXPECT_EQ(5, expected.size());
        for (int i = 0; i < 5; ++i) {
            EXPECT_EQ(expected[i],
                    evaluate(QString("getLight('[ChannelX]', '!fxButton%1');").arg(i)).toInt32());
        }
    }

    void CheckEnableLights(const std::vector<int>& expected) {
        EXPECT_EQ(4, expected.size());
        for (int i = 0; i < 4; ++i) {
            EXPECT_EQ(expected[i],
                    evaluate(QString("getLight('[Channel%1]', '!fxEnabled');")
                                     .arg(i + 1))
                            .toInt32());
        }
    }

    enum states {
        STATE_FILTER,
        STATE_EFFECT,
        STATE_FOCUS
    };
};

// Test tapping fx select buttons to toggle states -- Filter for filter state, any fx unit
// for effect state.
TEST_F(TraktorS3Test, FXSelectButtonSimple) {
    ASSERT_TRUE(evaluate(
            "var pressFx2 = { "
            "  group: '[ChannelX]', "
            "  name: '!fx2', "
            "  value: 1, "
            "}; "
            "var unpressFx2 = { "
            "  group: '[ChannelX]', "
            "  name: '!fx2', "
            "  value: 0, "
            "}; "
            "var pressFilter = { "
            "  group: '[ChannelX]', "
            "  name: '!fx0', "
            "  value: 1, "
            "}; "
            "var unpressFilter = { "
            "  group: '[ChannelX]', "
            "  name: '!fx0', "
            "  value: 0, "
            "}; ")
                        .isValid());

    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());

    // First try pressing a select button and releasing
    evaluate("TestOb.fxc.fxSelectHandler(pressFx2);");
    auto ret = evaluate("getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array1[5] = {false, false, true, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array1[i], ret.property(i).toBool());
    }
    SCOPED_TRACE("");
    CheckSelectLights({5, 15, 26, 35, 45});
    CheckEnableLights({25, 25, 25, 25});
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Now unpress select and release
    evaluate("TestOb.fxc.fxSelectHandler(unpressFx2);");
    ret = evaluate("getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array2[5] = {false, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array2[i], ret.property(i).toBool());
    }
    SCOPED_TRACE("");
    CheckSelectLights({5, 15, 26, 35, 45});
    CheckEnableLights({25, 25, 25, 25});
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Now press filter button and release
    evaluate("TestOb.fxc.fxSelectHandler(pressFilter);");
    ret = evaluate("getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array3[5] = {true, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array3[i], ret.property(i).toBool());
    }
    SCOPED_TRACE("");
    CheckSelectLights({6, 15, 25, 35, 45});
    CheckEnableLights({25, 25, 25, 25});
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());

    evaluate("TestOb.fxc.fxSelectHandler(unpressFilter);");
    ret = evaluate("getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array4[5] = {false, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array4[i], ret.property(i).toBool());
    }
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());
}

// Hold FX button + tap effect enable focuses that effect.
TEST_F(TraktorS3Test, FXSelectFocusToggle) {
    ASSERT_TRUE(evaluate(
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };"
            "var pressFilter = { group: '[ChannelX]', name: '!fx0', value: 1 };"
            "var unpressFilter = { group: '[ChannelX]', name: '!fx0', value: 0 };")
                        .isValid());

    // Press FX2 and release
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    auto ret = evaluate("getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array2[5] = {false, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array2[i], ret.property(i).toBool());
    }
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());
    SCOPED_TRACE("");
    CheckSelectLights({5, 15, 26, 35, 45});
    CheckEnableLights({25, 25, 25, 25});

    // Press fx2 and enable2, focus third effect (channel 2 button is third button)
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable2);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt32());
    EXPECT_EQ(3,
            ControlObject::getControl(
                    ConfigKey("[EffectRack1_EffectUnit2]", "focused_effect"))
                    ->get());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());
    SCOPED_TRACE("");
    // CheckSelectLights({5, 15, 26, 35, 45});
    // CheckEnableLights({0, 10, 21, 30});

    // Press again, back to effect mode
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());
    SCOPED_TRACE("");
    CheckSelectLights({5, 15, 26, 35, 45});
    CheckEnableLights({25, 25, 25, 25});

    // Press 3, effect
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx3); "
            "TestOb.fxc.fxSelectHandler(unpressFx3);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(3, evaluate("getActiveFx();").toInt32());
    SCOPED_TRACE("");
    CheckSelectLights({5, 15, 25, 36, 45});
    CheckEnableLights({0, 10, 20, 30});

    // Press 2, press 2, press filter = filter
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);"
            "TestOb.fxc.fxSelectHandler(pressFilter); "
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());

    // Hold filter, press enable, noop
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFilter); "
            "TestOb.fxc.fxEnableHandler(pressFxEnable2);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable2);"
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());
}

// Test Enable buttons + FX Select buttons to enable/disable fx units per channel.
// This is only available during Filter state.
TEST_F(TraktorS3Test, FXEnablePlusFXSelect) {
    // IMPORTANT: Channel 1 is the second button (CABD mapping).
    ASSERT_TRUE(evaluate(
            "var pressFxEnable1 = { group: '[Channel1]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable1 = { group: '[Channel1]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };"
            "var pressFilter = { group: '[ChannelX]', name: '!fx0', value: 1 };"
            "var unpressFilter = { group: '[ChannelX]', name: '!fx0', value: 0 };")
                        .isValid());

    // Press FXEnable 1 and release
    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1); ");
    auto ret = evaluate("getEnablePressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array1[4] = {true, false, false, false};
    for (int i = 0; i < 4; ++i) {
        QString group = QString("[Channel%1]").arg(i + 1);
        EXPECT_TRUE(ret.property(group).isValid());
        EXPECT_EQ(expected_array1[i], ret.property(group).toBool());
    }

    evaluate(
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1); ");
    ret = evaluate("getEnablePressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array2[5] = {false, false, false, false};
    for (int i = 1; i <= 4; ++i) {
        QString group = QString("[Channel%1]").arg(i);
        EXPECT_TRUE(ret.property(group).isValid());
        EXPECT_EQ(expected_array2[i], ret.property(group).toBool());
    }

    // Press enable 1, fx2, should enable effect unit 2 for channel 1
    // Keep enable pressed
    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");

    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                  "group_[Channel1]_enable"))
                        ->get());
    SCOPED_TRACE("");
    CheckEnableLights({1, 10, 20, 30});
    CheckSelectLights({5, 16, 25, 35, 45});

    // Press enable fx2 again, should disable effect unit 2 for channel 1
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");

    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                   "group_[Channel1]_enable"))
                         ->get());

    // Unpress fxenable, back where we started.
    evaluate(
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1); ");
    ret = evaluate("getEnablePressed();");
    ASSERT_TRUE(ret.isValid());

    for (int i = 1; i <= 4; ++i) {
        QString group = QString("[Channel%1]").arg(i);
        EXPECT_TRUE(ret.property(group).isValid());
        EXPECT_EQ(expected_array2[i], ret.property(group).toBool());
    }

    // If we're not in filter mode, fxenable doesn't cause us to enable/disable units
    // (this would enable/disable the effectunit, but that's tested elsewhere)
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx3);"
            "TestOb.fxc.fxSelectHandler(unpressFx3);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);"
            "TestOb.fxc.fxSelectHandler(pressFx2);");
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                   "group_[Channel1]_enable"))
                         ->get());
}

// In FX Mode, the FX Enable buttons toggle effect units
TEST_F(TraktorS3Test, FXModeFXEnable) {
    // IMPORTANT: Channel 3 is the first button.
    ASSERT_TRUE(evaluate(
            "var pressFxEnable1 = { group: '[Channel3]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable1 = { group: '[Channel3]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };")
                        .isValid());

    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "enabled"))
                         ->get());

    // Enable effect mode
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    SCOPED_TRACE("");
    CheckEnableLights({25, 25, 25, 25});
    CheckSelectLights({5, 15, 26, 35, 45});

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);");

    // Effect Unit 1 is toggled
    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                  "enabled"))
                        ->get());
    SCOPED_TRACE("");
    // Channel 3 is the first button
    CheckEnableLights({25, 25, 26, 25});
    CheckSelectLights({5, 15, 26, 35, 45});

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);");

    // Effect Unit 1 is toggled
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "enabled"))
                         ->get());
    SCOPED_TRACE("");
    CheckEnableLights({25, 25, 25, 25});
    CheckSelectLights({5, 15, 26, 35, 45});
}

// In Focus Mode, the FX Enable buttons toggle effect parameter values
TEST_F(TraktorS3Test, FocusModeFXEnable) {
    // IMPORTANT: Channel 3 is the first button.
    ASSERT_TRUE(evaluate(
            "var pressFxEnable1 = { group: '[Channel3]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable1 = { group: '[Channel3]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };")
                        .isValid());

    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "enabled"))
                         ->get());

    // Enable focus mode for fx2, effect 1.
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt32());

    // Effect1 in Unit 2 is toggled
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "parameter1"))
                         ->get());

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);");

    // Effect Unit 1 is toggled
    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                  "parameter1"))
                        ->get());

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);");

    // Effect Unit 1 is toggled
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "parameter1"))
                         ->get());
}

// Test knob behavior in different states
TEST_F(TraktorS3Test, KnobTest) {
    ASSERT_TRUE(evaluate(
            "var pressFxEnable1 = { group: '[Channel1]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable1 = { group: '[Channel1]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };"
            "var pressFilter = { group: '[ChannelX]', name: '!fx0', value: 1 };"
            "var unpressFilter = { group: '[ChannelX]', name: '!fx0', value: 0 };")
                        .isValid());

    // STATE_FILTER: knobs control quickeffects
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFilter);"
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());

    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel1]', name: '!fxKnob', "
            "value: 0.75*4095 } );");
    EXPECT_FLOAT_EQ(0.75,
            ControlObject::getControl(
                    ConfigKey("[QuickEffectRack1_[Channel1]]", "super1"))
                    ->get());

    // STATE_EFFECT: knobs control effectunit meta knobs
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());

    // Note, Channel2 is the third knob
    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel2]', name: '!fxKnob', "
            "value: 0.62*4095 } );");
    EXPECT_FLOAT_EQ(0.62,
            ControlObject::getControl(
                    ConfigKey("[EffectRack1_EffectUnit2_Effect3]", "meta"))
                    ->get());

    // Knob 4 is the mix knob
    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel4]', name: '!fxKnob', "
            "value: 0.22*4095 } );");
    EXPECT_FLOAT_EQ(0.22,
            ControlObject::getControl(
                    ConfigKey("[EffectRack1_EffectUnit2]", "mix"))
                    ->get());

    // Set state to Focus -- knobs control effect parameters
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt32());

    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel3]', name: '!fxKnob', "
            "value: 0.12*4095 } );");
    EXPECT_FLOAT_EQ(0.12,
            ControlObject::getControl(
                    ConfigKey(
                            "[EffectRack1_EffectUnit2_Effect2]", "parameter1"))
                    ->get());
}