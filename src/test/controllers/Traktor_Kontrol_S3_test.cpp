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
        m_pRack = m_pEffectsManager->addStandardEffectRack();

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
                "};");
    }

    enum states {
        STATE_FILTER,
        STATE_EFFECT,
        STATE_FOCUS
    };
    StandardEffectRackPointer m_pRack;
};

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

TEST_F(TraktorS3Test, FXSelectFocusToggle) {
    ASSERT_TRUE(evaluate(
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

    // Press 2 again, focus
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Press again, back to effect mode
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Press 2 again, focus
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Press 3, effect
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx3); "
            "TestOb.fxc.fxSelectHandler(unpressFx3);");
    EXPECT_EQ(STATE_EFFECT, evaluate("getState();").toInt32());
    EXPECT_EQ(3, evaluate("getActiveFx();").toInt32());

    // Press 2, press 2, press filter = filter
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);"
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);"
            "TestOb.fxc.fxSelectHandler(pressFilter); "
            "TestOb.fxc.fxSelectHandler(unpressFilter);");
    EXPECT_EQ(STATE_FILTER, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());
}

TEST_F(TraktorS3Test, FXEnablePlusFXSelect) {
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
    evaluate(
            "TestOb.fxc.fxEnableHandler(pressFxEnable1);"
            "TestOb.fxc.fxSelectHandler(pressFx2);"
            "TestOb.fxc.fxSelectHandler(unpressFx2);");

    EXPECT_TRUE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                  "group_[Channel1]_enable"))
                        ->get());

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
            "TestOb.fxc.fxSelectHandler(pressFx2);");
    EXPECT_FALSE(ControlObject::getControl(ConfigKey("[EffectRack1_EffectUnit2]",
                                                   "group_[Channel1]_enable"))
                         ->get());
}