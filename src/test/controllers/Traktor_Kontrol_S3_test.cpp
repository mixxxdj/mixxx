#include <QThread>
#include <QtDebug>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/controllerdebug.h"
#include "controllers/controllerengine.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/memory.h"
#include "util/time.h"

class ControllerTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        QThread::currentThread()->setObjectName("Main");
        m_pCEngine = new ControllerEngine(nullptr, config());
        ControllerDebug::enable();
        m_pCEngine->setPopups(false);
    }

    void TearDown() override {
        m_pCEngine->gracefulShutdown();
        delete m_pCEngine;
        mixxx::Time::setTestMode(false);
    }

    QScriptValue evaluate(const QString& program) {
        return m_pCEngine->evaluateDirect(program);
    }

    // Executes a text string of javascript, can contain newlines.
    // Returns the value resulting from the execution.
    QScriptValue executeScript(const QString& scriptText) {
        ScopedTemporaryFile script(makeTemporaryFile(scriptText));
        QScriptValue ret;
        m_pCEngine->evaluateWithReturn(script->fileName(), &ret);
        return ret;
    }

    void processEvents() {
        // QCoreApplication::processEvents() only processes events that were
        // queued when the method was called. Hence, all subsequent events that
        // are emitted while processing those queued events will not be
        // processed and are enqueued for the next event processing cycle.
        // Calling processEvents() twice ensures that at least all queued and
        // the next round of emitted events are processed.
        application()->processEvents();
        application()->processEvents();
    }

    ControllerEngine* m_pCEngine;
};

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
        executeScript(
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

  private:
    const QString m_sScriptFile = "./res/controllers/Traktor-Kontrol-S3-hid-scripts.js";
    bool m_bStateFnDefined = false;
    bool m_bAciveFXFnDefined = false;
};

TEST_F(TraktorS3Test, FXSelectFX) {
    ASSERT_TRUE(executeScript(
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

    // QScriptValue ret2 = executeScript("getState();");
    // qDebug() << "hmm what about execute" << ret2.toString();

    QScriptValue ret3 = evaluate("getState();");
    qDebug() << "even easier?" << ret3.toString();

    EXPECT_EQ(0, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());

    // First try pressing a select button and releasing
    executeScript("TestOb.fxc.fxSelectHandler(pressFx2);");
    auto ret = evaluate("getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array1[5] = {false, false, true, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array1[i], ret.property(i).toBool());
    }
    auto what = evaluate("getState();");
    EXPECT_TRUE(what.isNumber());
    EXPECT_EQ(1, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Now unpress select and release
    ret = executeScript(
            "TestOb.fxc.fxSelectHandler(unpressFx2);"
            "getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array2[5] = {false, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array2[i], ret.property(i).toBool());
    }
    EXPECT_EQ(1, evaluate("getState();").toInt32());
    EXPECT_EQ(2, evaluate("getActiveFx();").toInt32());

    // Now press filter button and release
    ret = executeScript(
            "TestOb.fxc.fxSelectHandler(pressFilter); "
            "getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array3[5] = {true, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array3[i], ret.property(i).toBool());
    }
    EXPECT_EQ(0, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());

    ret = executeScript(
            "TestOb.fxc.fxSelectHandler(unpressFilter); "
            "getSelectPressed();");
    ASSERT_TRUE(ret.isValid());

    bool expected_array4[5] = {false, false, false, false, false};
    for (int i = 0; i < 5; ++i) {
        EXPECT_TRUE(ret.property(i).isValid());
        EXPECT_EQ(expected_array4[i], ret.property(i).toBool());
    }
    EXPECT_EQ(0, evaluate("getState();").toInt32());
    EXPECT_EQ(0, evaluate("getActiveFx();").toInt32());
}
