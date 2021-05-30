#include <QThread>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/controllerdebug.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "controllers/softtakeover.h"
#include "effects/effectchain.h"
#include "effects/effectslot.h"
#include "effects/effectsmanager.h"
#include "test/baseeffecttest.h"
#include "test/controllers/controllertest.h"
#include "test/signalpathtest.h"

class TraktorS3Test : public ControllerTest {
  protected:
    TraktorS3Test() {
        m_pTestBackend = new TestEffectBackend();
        m_pEffectsManager->addEffectsBackend(m_pTestBackend);
    }

    void SetUp() override {
        ControllerTest::SetUp();

        // Load a few effects so we can interact with them.
        EffectManifestPointer pManifest(new EffectManifest());
        pManifest->setId("org.mixxx.test.effect");
        pManifest->setName("Test Effect1");
        pManifest->addParameter();
        registerTestEffect(pManifest, false);
        EffectPointer pEffect = m_pEffectsManager->instantiateEffect(pManifest->id());

        EffectChainPointer pChain(new EffectChain(m_pEffectsManager,
                "org.mixxx.test.chain1"));

        for (int chain = 0; chain < 2; ++chain) {
            auto chainSlot = m_pRack->getEffectChainSlot(chain);
            chainSlot->loadEffectChainToSlot(pChain);
            for (int effect = 0; effect < 2; ++effect) {
                auto effectSlot = chainSlot->getEffectSlot(effect);
                effectSlot->loadEffect(pEffect, false);
            }
        }

        const QString commonScript = "./res/controllers/common-controller-scripts.js";
        const QString hidScript = "./res/controllers/common-hid-packet-parser.js";
        const QString scriptFile = "./res/controllers/Traktor-Kontrol-S3-hid-scripts.js";
        ASSERT_TRUE(evaluateScriptFile(commonScript));
        ASSERT_TRUE(evaluateScriptFile(hidScript));
        ASSERT_TRUE(evaluateScriptFile(scriptFile));

        // Create useful objects and getters
        ASSERT_FALSE(evaluate(R"(
                var TestOb = {};
                TestOb.fxc = new TraktorS3.FXControl();
                var getState = function() {
                 return TestOb.fxc.currentState;
                };
                var getActiveFx = function() {
                 return TestOb.fxc.activeFX;
                };
                var getSelectPressed = function() {
                 return TestOb.fxc.selectPressed;
                };
                var getEnablePressed = function() {
                 return TestOb.fxc.enablePressed;
                };
                // Mock out shift key.
                TestOb.shiftPressed = false;
                TraktorS3.anyShiftPressed = function() {
                  return TestOb.shiftPressed;
                } )")
                             .isError());

        // Mock out functions and controller for testing lights
        ASSERT_FALSE(evaluate(R"(
                TraktorS3.FXControl.prototype.getFXSelectLEDValue = function(fxNumber, status) {
                    return fxNumber*10 + status;
                };
                TraktorS3.FXControl.prototype.getChannelColor = function(group, status) {
                    return this.channelToIndex(group)*10 + status;
                };
                // stub out state changer so we don't do time-based blinking
                TraktorS3.FXControl.prototype.changeState = function(newState) {
                    this.currentState = newState;
                };
                var setBlink = function(state) {
                    TestOb.fxc.controller.focusBlinkState = state;
                };
                TestOb.fxc.controller = new function() {
                    this.batchingOutputs = false;
                    this.lightMap = {};
                    this.hid = {};
                    this.hid.setOutput = function(group, key, value, batching) {
                        if (!(group in TestOb.fxc.controller.lightMap)) {
                            TestOb.fxc.controller.lightMap[group] = {};
                        }
                        // HIDDebug('setting light: ' + group + ' ' + key + ' ' + value);
                        TestOb.fxc.controller.lightMap[group][key] = value;
                    };
                };
                var getLight = function(group, key) {
                    if (!(group in TestOb.fxc.controller.lightMap)) {
                        return undefined;
                    }
                    // HIDDebug('getting light ' + group + ' ' + key + ' ' +
                    //     TestOb.fxc.controller.lightMap[group][key]);
                    return TestOb.fxc.controller.lightMap[group][key];
                };)")
                             .isError());
    }

    void registerTestEffect(EffectManifestPointer pManifest, bool willAddToEngine) {
        MockEffectProcessor* pProcessor = new MockEffectProcessor();
        MockEffectInstantiator* pInstantiator = new MockEffectInstantiator();

        if (willAddToEngine) {
            EXPECT_CALL(*pInstantiator, instantiate(_, _))
                    .Times(1)
                    .WillOnce(Return(pProcessor));
        }

        m_pTestBackend->registerEffect(pManifest->id(),
                pManifest,
                EffectInstantiatorPointer(pInstantiator));
    }

    void CheckSelectPressed(const std::vector<bool>& expected, const QJSValue& got) {
        EXPECT_EQ(5, expected.size());
        for (int i = 0; i < 0; ++i) {
            EXPECT_TRUE(got.property(i).isBool());
            EXPECT_EQ(expected[i], got.property(i).toBool());
        }
    }

    // Checks that the correct enabled buttons are pressed. The expected values are in
    // physical order, not channel order.
    void CheckEnabledPressed(const std::vector<bool>& expected, const QJSValue& got) {
        EXPECT_EQ(4, expected.size());
        EXPECT_TRUE(got.property("[Channel3]").isBool());
        EXPECT_EQ(expected[0], got.property("[Channel3]").toBool());
        EXPECT_TRUE(got.property("[Channel1]").isBool());
        EXPECT_EQ(expected[1], got.property("[Channel1]").toBool());
        EXPECT_TRUE(got.property("[Channel2]").isBool());
        EXPECT_EQ(expected[2], got.property("[Channel2]").toBool());
        EXPECT_TRUE(got.property("[Channel4]").isBool());
        EXPECT_EQ(expected[3], got.property("[Channel4]").toBool());
    }

    // For the list of lights, the tens digit is the effect number or channel number, and the
    // ones digit is 0 for off, 1 for dim, and 2 for bright.
    void CheckSelectLights(const std::vector<int>& expected) {
        EXPECT_EQ(5, expected.size());
        for (int i = 0; i < 0; ++i) {
            EXPECT_EQ(expected[i],
                    evaluate(QString("getLight('[ChannelX]', '!fxButton%1');").arg(i)).toInt())
                    << "failed on select light: " << i;
        }
    }

    // Checks that the enable lights are lit correctly. The expected values are in
    // physical order, not channel order.
    // For the list of lights, the tens digit is the effect number or channel number, and the
    // ones digit is 0 for off, 1 for dim, and 2 for bright.
    void CheckEnableLights(const std::vector<int>& expected) {
        EXPECT_EQ(4, expected.size());
        EXPECT_EQ(expected[0],
                evaluate(QString("getLight('[Channel3]', '!fxEnabled');"))
                        .toInt());
        EXPECT_EQ(expected[1],
                evaluate(QString("getLight('[Channel1]', '!fxEnabled');"))
                        .toInt());
        EXPECT_EQ(expected[2],
                evaluate(QString("getLight('[Channel2]', '!fxEnabled');"))
                        .toInt());
        EXPECT_EQ(expected[3],
                evaluate(QString("getLight('[Channel4]', '!fxEnabled');"))
                        .toInt());
    }

    enum states {
        STATE_FILTER,
        STATE_EFFECT_INIT,
        STATE_EFFECT,
        STATE_FOCUS
    };

    TestEffectBackend* m_pTestBackend;
};

// Test tapping fx select buttons to toggle states -- Filter for filter state, any fx unit
// for effect state.
TEST_F(TraktorS3Test, FXSelectButtonSimple) {
    evaluate(
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
            "}; ");

    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt());

    // First try pressing a select button and releasing
    ASSERT_FALSE(evaluate("TestOb.fxc.fxSelectHandler(pressFx2);").isError());

    auto ret = evaluate("getSelectPressed();");
    {
        SCOPED_TRACE("");
        CheckSelectPressed({false, false, true, false, false}, ret);
        CheckSelectLights({0, 10, 22, 30, 40});
        CheckEnableLights({21, 21, 20, 20});
    }
    EXPECT_EQ(STATE_EFFECT_INIT, evaluate("getState();").toInt());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt());

    // Now unpress select and release
    evaluate("TestOb.fxc.fxSelectHandler(unpressFx2);");
    ret = evaluate("getSelectPressed();");
    {
        SCOPED_TRACE("");
        CheckSelectPressed({false, false, false, false, false}, ret);
        CheckSelectLights({0, 10, 21, 30, 40});
        CheckEnableLights({21, 21, 20, 20});
    }
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt());

    // Now press filter button and release
    evaluate("TestOb.fxc.fxSelectHandler(pressFilter);");
    ret = evaluate("getSelectPressed();");
    {
        SCOPED_TRACE("");
        CheckSelectPressed({true, false, false, false, false}, ret);
        CheckSelectLights({2, 11, 21, 31, 41});
        CheckEnableLights({11, 21, 31, 41});
    }
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt());

    evaluate("TestOb.fxc.fxSelectHandler(unpressFilter);");
    ret = evaluate("getSelectPressed();");
    {
        SCOPED_TRACE("");
        CheckSelectPressed({false, false, false, false, false}, ret);
    }
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt());
}

// Hold FX button + tap effect enable focuses that effect.
TEST_F(TraktorS3Test, FXSelectFocusToggle) {
    evaluate(
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };"
            "var pressFilter = { group: '[ChannelX]', name: '!fx0', value: 1 };"
            "var unpressFilter = { group: '[ChannelX]', name: '!fx0', value: 0 };");

    // Press FX2 and release
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    auto ret = evaluate("getSelectPressed();");

    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt());
    {
        SCOPED_TRACE("");
        CheckSelectPressed({false, false, false, false, false}, ret);
        CheckSelectLights({0, 10, 21, 30, 40});
        CheckEnableLights({21, 21, 20, 20});
    }

    // Press fx2 and enable2, focus third effect (channel 2 button is third button)
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable2);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt());
    EXPECT_EQ(3,
            ControlObject::getControl(
                    ConfigKey("[EffectRack1_EffectUnit2]", "focused_effect"))
                    ->get());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt());
    {
        SCOPED_TRACE("");
        CheckSelectLights({0, 10, 22, 30, 40});
        CheckEnableLights({20, 20, 20, 20});
    }
    evaluate("setBlink(true);");
    {
        SCOPED_TRACE("");
        CheckSelectLights({0, 10, 21, 30, 40});
    }

    // Press again, back to effect mode
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt());
    {
        SCOPED_TRACE("");
        CheckSelectLights({0, 10, 21, 30, 40});
        CheckEnableLights({21, 21, 20, 20});
    }

    // Press 3, effect
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx3); "
            "TestOb.fxc.fxSelectHandler(unpressFx3);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt());
    EXPECT_EQ(3, evaluate("getActiveFx();").toInt());
    {
        SCOPED_TRACE("");
        CheckSelectLights({0, 10, 20, 31, 40});
        CheckEnableLights({30, 30, 30, 30});
    }

    // Press 2, press 2, press filter = filter
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);"
            "TestOb.fxc.fxSelectHandler(pressFilter); "
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt());

    // Hold filter, press enable, noop
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFilter); "
            "TestOb.fxc.fxEnableHandler(pressFxEnable2);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable2);"
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt());
}

// Test Enable buttons + FX Select buttons to enable/disable fx units per channel.
// This is only available during Filter state.
TEST_F(TraktorS3Test, FXEnablePlusFXSelect) {
    evaluate(
            "var pressFxEnable3 = { group: '[Channel3]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable3 = { group: '[Channel3]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };"
            "var pressFilter = { group: '[ChannelX]', name: '!fx0', value: 1 };"
            "var unpressFilter = { group: '[ChannelX]', name: '!fx0', value: 0 };");

    // For some reason, some effects start out enabled.
    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit1]",
                                                  "group_[Channel1]_enable"))
                        ->get());
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                   "group_[Channel1]_enable"))
                         ->get());
    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit3]",
                                                  "group_[Channel3]_enable"))
                        ->get());

    // Press FXEnable for Channel 3 and release
    evaluate("TestOb.fxc.fxEnableHandler(pressFxEnable3);");
    auto ret = evaluate("getEnablePressed();");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    {
        SCOPED_TRACE("");
        CheckEnabledPressed({true, false, false, false}, ret);
        CheckEnableLights({12, 21, 31, 41});
        CheckSelectLights({0, 10, 20, 32, 40});
    }

    evaluate(
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3); ");
    ret = evaluate("getEnablePressed();");
    {
        SCOPED_TRACE("");
        CheckEnabledPressed({false, false, false, false}, ret);
        CheckEnableLights({11, 21, 31, 41});
        CheckSelectLights({2, 11, 21, 31, 41});
    }

    // Go back to filter mode. Press enable ch3, fx2, should enable effect unit 2 for channel 3
    // Keep enable pressed
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFilter);"
            "TestOb.fxc.fxSelectHandler(unpressFilter);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");

    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                  "group_[Channel3]_enable"))
                        ->get());
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    {
        SCOPED_TRACE("");
        CheckEnableLights({12, 21, 31, 41});
        CheckSelectLights({0, 10, 22, 32, 40});
    }

    // Press enable fx2 again, should enable effect unit 2 for channel 1
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");

    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                   "group_[Channel3]_enable"))
                         ->get());
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());
    {
        SCOPED_TRACE("");
        CheckEnableLights({12, 21, 31, 41});
        CheckSelectLights({0, 10, 20, 32, 40});
    }

    // Unpress fxenable, back where we started.
    evaluate(
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3); ");
    ret = evaluate("getEnablePressed();");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());

    {
        SCOPED_TRACE("");
        CheckEnabledPressed({false, false, false, false}, ret);
        CheckEnableLights({11, 21, 31, 41});
        CheckSelectLights({2, 11, 21, 31, 41});
    }

    // If we're not in filter mode, fxenable doesn't cause us to enable/disable units
    // (this would enable/disable the effectunit, but that's tested elsewhere)
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx3);"
            "TestOb.fxc.fxSelectHandler(unpressFx3);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3);");
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                   "group_[Channel3]_enable"))
                         ->get());
}

// In FX Mode, the FX Enable buttons toggle effect units
TEST_F(TraktorS3Test, FXModeFXEnable) {
    // IMPORTANT: Channel 3 is the first button.
    evaluate(
            "var pressFxEnable3 = { group: '[Channel3]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable3 = { group: '[Channel3]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };");

    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "enabled"))
                         ->get());

    // Enable effect mode
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt());
    {
        SCOPED_TRACE("");
        CheckEnableLights({21, 21, 20, 20});
        CheckSelectLights({0, 10, 21, 30, 40});
    }

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3);");

    // Effect Unit 1 is toggled
    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                  "enabled"))
                        ->get());
    {
        SCOPED_TRACE("");
        // Channel 3 is the first button
        CheckEnableLights({22, 22, 20, 20});
        CheckSelectLights({0, 10, 21, 30, 40});
    }

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3);");

    // Effect Unit 1 is toggled
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "enabled"))
                         ->get());
    {
        SCOPED_TRACE("");
        CheckEnableLights({21, 21, 20, 20});
        CheckSelectLights({0, 10, 21, 30, 40});
    }
}

// In Focus Mode, the FX Enable buttons toggle effect parameter values
TEST_F(TraktorS3Test, FocusModeFXEnable) {
    // IMPORTANT: Channel 3 is the first button.
    evaluate(
            "var pressFxEnable3 = { group: '[Channel3]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable3 = { group: '[Channel3]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };");

    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "enabled"))
                         ->get());

    // Enable focus mode for fx2, effect 1.
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt());

    // Effect1 in Unit 2 is toggled
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "button_parameter1"))
                         ->get());
    {
        SCOPED_TRACE("");
        CheckEnableLights({20, 20, 20, 20});
        CheckSelectLights({0, 10, 22, 30, 40});
    }
    evaluate("setBlink(true);");
    {
        SCOPED_TRACE("");
        CheckSelectLights({0, 10, 22, 31, 40});
    }

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3);");

    // Effect 1 button parameter 1 is toggled
    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                  "button_parameter1"))
                        ->get());
    {
        SCOPED_TRACE("");
        CheckEnableLights({22, 20, 20, 20});
        CheckSelectLights({0, 10, 22, 31, 40});
    }
    evaluate("setBlink(true);");
    {
        SCOPED_TRACE("");
        CheckEnableLights({22, 20, 20, 20});
        CheckSelectLights({0, 10, 22, 30, 40});
    }

    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable3);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable3);");

    // Effect button parameter toggled back
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2_Effect1]",
                                                   "button_parameter1"))
                         ->get());
    {
        SCOPED_TRACE("");
        CheckEnableLights({20, 20, 20, 20});
        CheckSelectLights({0, 10, 22, 30, 40});
    }
}

// Test knob behavior in different states
TEST_F(TraktorS3Test, KnobTest) {
    evaluate(
            "var pressFxEnable1 = { group: '[Channel1]',  name: '!fxEnabled',  value: 1 };"
            "var unpressFxEnable1 = { group: '[Channel1]', name: '!fxEnabled', value: 0 };"
            "var pressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 1 };"
            "var unpressFxEnable2 = { group: '[Channel2]', name: '!fxEnabled', value: 0 };"
            "var pressFx2 = { group: '[ChannelX]', name: '!fx2', value: 1 };"
            "var unpressFx2 = { group: '[ChannelX]', name: '!fx2', value: 0 };"
            "var pressFx3 = { group: '[ChannelX]', name: '!fx3', value: 1 };"
            "var unpressFx3 = { group: '[ChannelX]', name: '!fx3', value: 0 };"
            "var pressFilter = { group: '[ChannelX]', name: '!fx0', value: 1 };"
            "var unpressFilter = { group: '[ChannelX]', name: '!fx0', value: 0 };");

    // STATE_FILTER: knobs control quickeffects
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFilter);"
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt());

    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel1]', name: '!fxKnob', "
            "value: 0.75*4095 } );");
    EXPECT_DOUBLE_EQ(0.75,
            ControlObject::getControl(
                    ConfigKey("[QuickEffectRack1_[Channel1]]", "super1"))
                    ->get());

    // STATE_EFFECT: knobs control effectunit meta knobs
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt());

    // Note, Channel2 is the third knob
    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel2]', name: '!fxKnob', "
            "value: 0.62*4095 } );");
    EXPECT_DOUBLE_EQ(0.62,
            ControlObject::getControl(
                    ConfigKey("[EffectRack1_EffectUnit2_Effect3]", "meta"))
                    ->get());

    // Knob 4 is the mix knob
    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel4]', name: '!fxKnob', "
            "value: 0.22*4095 } );");
    EXPECT_DOUBLE_EQ(0.22,
            ControlObject::getControl(
                    ConfigKey("[EffectRack1_EffectUnit2]", "mix"))
                    ->get());

    // Set state to Focus -- knobs control effect parameters
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxEnableHandler(unpressFxEnable1);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt());

    evaluate(
            "TestOb.fxc.fxKnobHandler( { group: '[Channel3]', name: '!fxKnob', "
            "value: 0.12*4095 } );");
    EXPECT_DOUBLE_EQ(0.12,
            ControlObject::getControl(
                    ConfigKey(
                            "[EffectRack1_EffectUnit2_Effect2]", "parameter1"))
                    ->get());
}
