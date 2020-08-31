#include <QThread>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/controllerdebug.h"
#include "controllers/controllerengine.h"
#include "controllers/softtakeover.h"
#include "test/controllers/controllertest.h"

class TraktorS3Test : public ControllerTest {
  protected:
    void SetUp() override {
        ControllerTest::SetUp();
        QString hidScript = "./res/controllers/common-hid-packet-parser.js";
        ASSERT_TRUE(m_pCEngine->evaluate(hidScript));
        ASSERT_FALSE(m_pCEngine->hasErrors(hidScript));
        ASSERT_TRUE(m_pCEngine->evaluate(m_sScriptFile));
        ASSERT_FALSE(m_pCEngine->hasErrors(m_sScriptFile));

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

  private:
    const QString m_sScriptFile = "./res/controllers/Traktor-Kontrol-S3-hid-scripts.js";
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

    // Press again
    evaluate(
            "TestOb.fxc.fxSelectHandler(pressFx2); "
            "TestOb.fxc.fxSelectHandler(unpressFx2);");
    EXPECT_EQ(STATE_FOCUS, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());
}
