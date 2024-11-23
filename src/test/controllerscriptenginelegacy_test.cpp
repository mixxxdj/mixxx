#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QMetaEnum>
#include <QScopedPointer>
#include <QTemporaryFile>
#include <QThread>
#include <QtDebug>
#include <bit>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#ifdef MIXXX_USE_QML
#include <QQuickItem>

#include "controllers/controllerenginethreadcontrol.h"
#include "controllers/rendering/controllerrenderingengine.h"
#endif
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "controllers/softtakeover.h"
#include "helpers/log_test.h"
#include "preferences/usersettings.h"
#ifdef MIXXX_USE_QML
#include "qml/qmlmixxxcontrollerscreen.h"
#endif
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/time.h"

using ::testing::_;
using namespace std::chrono_literals;

typedef std::unique_ptr<QTemporaryFile> ScopedTemporaryFile;

const RuntimeLoggingCategory logger(QString("test").toLocal8Bit());

class ControllerScriptEngineLegacyTest : public ControllerScriptEngineLegacy, public MixxxTest {
  protected:
    ControllerScriptEngineLegacyTest()
            : ControllerScriptEngineLegacy(nullptr, logger) {
    }
    static ScopedTemporaryFile makeTemporaryFile(const QString& contents) {
        QByteArray contentsBa = contents.toLocal8Bit();
        ScopedTemporaryFile pFile = std::make_unique<QTemporaryFile>();
        pFile->open();
        pFile->write(contentsBa);
        pFile->close();
        return pFile;
    }

    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::addTestTime(10ms);
        QThread::currentThread()->setObjectName("Main");
        initialize();
    }

    void TearDown() override {
        mixxx::Time::setTestMode(false);
#ifdef MIXXX_USE_QML
        m_rootItems.clear();
#endif
    }

    bool evaluateScriptFile(const QFileInfo& scriptFile) {
        return ControllerScriptEngineLegacy::evaluateScriptFile(scriptFile);
    }

    QJSValue evaluate(const QString& code) {
        return jsEngine()->evaluate(code);
    }

    bool evaluateAndAssert(const QString& code) {
        return !evaluate(code).isError();
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

#ifdef MIXXX_USE_QML
    QHash<QString, std::shared_ptr<ControllerRenderingEngine>>& renderingScreens() {
        return m_renderingScreens;
    }

    std::unordered_map<QString,
            std::unique_ptr<mixxx::qml::QmlMixxxControllerScreen>>&
    rootItems() {
        return m_rootItems;
    }

    void testHandleScreen(
            const LegacyControllerMapping::ScreenInfo& screeninfo,
            const QImage& frame,
            const QDateTime& timestamp) {
        handleScreenFrame(screeninfo, frame, timestamp);
    }
#endif
};

TEST_F(ControllerScriptEngineLegacyTest, commonScriptHasNoErrors) {
    QFileInfo commonScript(config()->getResourcePath() +
            QStringLiteral("/controllers/common-controller-scripts.js"));
    EXPECT_TRUE(evaluateScriptFile(commonScript));
}

TEST_F(ControllerScriptEngineLegacyTest, setValue) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Test]', 'co', 1.0);"));
    EXPECT_DOUBLE_EQ(1.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, getValue_InvalidKey) {
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('', '');"));
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('', 'invalid');"));
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('[Invalid]', '');"));
}

TEST_F(ControllerScriptEngineLegacyTest, setValue_InvalidControl) {
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Nothing]', 'nothing', 1.0);"));
}

TEST_F(ControllerScriptEngineLegacyTest, getValue_InvalidControl) {
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('[Nothing]', 'nothing');"));
}

TEST_F(ControllerScriptEngineLegacyTest, setValue_IgnoresNaN) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    co->set(10.0);
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Test]', 'co', NaN);"));
    EXPECT_DOUBLE_EQ(10.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, getSetValue) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    EXPECT_TRUE(
            evaluateAndAssert("engine.setValue('[Test]', 'co', "
                              "engine.getValue('[Test]', 'co') + 1);"));
    EXPECT_DOUBLE_EQ(1.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, setParameter) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 1.0);"));
    EXPECT_DOUBLE_EQ(10.0, co->get());
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 0.0);"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 0.5);"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, setParameter_OutOfRange) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 1000);"));
    EXPECT_DOUBLE_EQ(10.0, co->get());
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', -1000);"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, setParameter_NaN) {
    // Test that NaNs are ignored.
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', NaN);"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, getSetParameter) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    EXPECT_TRUE(evaluateAndAssert(
            "engine.setParameter('[Test]', 'co', "
            "  engine.getParameter('[Test]', 'co') + 0.1);"));
    EXPECT_DOUBLE_EQ(2.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, softTakeover_setValue) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    co->setParameter(0.0);
    EXPECT_TRUE(evaluateAndAssert(
            "engine.softTakeover('[Test]', 'co', true);"
            "engine.setValue('[Test]', 'co', 0.0);"));
    // The first set after enabling is always ignored.
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Time elapsed is not greater than the threshold, so we do not ignore this
    // set.
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Test]', 'co', -10.0);"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Advance time to 2x the threshold.
    SoftTakeover::TestAccess::advanceTimePastThreshold();

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Ignore the change since it occurred after the threshold and is too large.
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Test]', 'co', -10.0);"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, softTakeover_setParameter) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    co->setParameter(0.0);
    EXPECT_TRUE(evaluateAndAssert(
            "engine.softTakeover('[Test]', 'co', true);"
            "engine.setParameter('[Test]', 'co', 1.0);"));
    // The first set after enabling is always ignored.
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Time elapsed is not greater than the threshold, so we do not ignore this
    // set.
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 0.0);"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    SoftTakeover::TestAccess::advanceTimePastThreshold();

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Ignore the change since it occurred after the threshold and is too large.
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 0.0);"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, softTakeover_ignoreNextValue) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    co->setParameter(0.0);
    EXPECT_TRUE(evaluateAndAssert(
            "engine.softTakeover('[Test]', 'co', true);"
            "engine.setParameter('[Test]', 'co', 1.0);"));
    // The first set after enabling is always ignored.
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    EXPECT_TRUE(evaluateAndAssert("engine.softTakeoverIgnoreNextValue('[Test]', 'co');"));

    // We would normally allow this set since it is below the time threshold,
    // but we are ignoring the next value.
    EXPECT_TRUE(evaluateAndAssert("engine.setParameter('[Test]', 'co', 0.0);"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, reset) {
    // Test that NaNs are ignored.
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
            -10.0,
            10.0);
    co->setParameter(1.0);
    EXPECT_TRUE(evaluateAndAssert("engine.reset('[Test]', 'co');"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, log) {
    EXPECT_TRUE(evaluateAndAssert("engine.log('Test that logging works.');"));
}

TEST_F(ControllerScriptEngineLegacyTest, trigger) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "var connection = engine.connectControl('[Test]', 'co', reaction);"
            "engine.trigger('[Test]', 'co');"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

// ControllerEngine::connectControl has a lot of quirky, inconsistent legacy behaviors
// depending on how it is invoked, so we need a lot of tests to make sure old scripts
// do not break.

TEST_F(ControllerScriptEngineLegacyTest, connectControl_ByString) {
    // Test that connecting and disconnecting by function name works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "engine.connectControl('[Test]', 'co', 'reaction');"
            "engine.trigger('[Test]', 'co');"
            "function disconnect() { "
            "  engine.connectControl('[Test]', 'co', 'reaction', 1);"
            "  engine.trigger('[Test]', 'co'); }"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_TRUE(evaluateAndAssert("disconnect();"));
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectControl_ByStringForbidDuplicateConnections) {
    // Test that connecting a control to a callback specified by a string
    // does not make duplicate connections. This behavior is inconsistent
    // with the behavior when specifying a callback as a function, but
    // this is how it has been done, so keep the behavior to ensure old scripts
    // do not break.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "engine.connectControl('[Test]', 'co', 'reaction');"
            "engine.connectControl('[Test]', 'co', 'reaction');"
            "engine.trigger('[Test]', 'co');"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest,
        connectControl_ByStringRedundantConnectionObjectsAreNotIndependent) {
    // Test that multiple connections are not allowed when passing
    // the callback to engine.connectControl as a function name string.
    // This is weird and inconsistent, but it is how it has been done,
    // so keep this behavior to make sure old scripts do not break.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    QString script(
            "var incrementCounterCO = function () {"
            "  var counter = engine.getValue('[Test]', 'counter');"
            "  engine.setValue('[Test]', 'counter', counter + 1);"
            "};"
            "var connection1 = engine.connectControl('[Test]', 'co', 'incrementCounterCO');"
            // Make a second connection with the same ControlObject
            // to check that disconnecting one does not disconnect both.
            "var connection2 = engine.connectControl('[Test]', 'co', 'incrementCounterCO');"
            "function changeTestCoValue() {"
            "  var testCoValue = engine.getValue('[Test]', 'co');"
            "  engine.setValue('[Test]', 'co', testCoValue + 1);"
            "};"
            "function disconnectConnection2() {"
            "  connection2.disconnect();"
            "};");

    evaluateAndAssert(script);
    EXPECT_TRUE(evaluateAndAssert(script));
    evaluateAndAssert("changeTestCoValue()");
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_EQ(1.0, counter->get());

    evaluateAndAssert("disconnectConnection2()");
    // The connection objects should refer to the same connection,
    // so disconnecting one should disconnect both.
    evaluateAndAssert("changeTestCoValue()");
    processEvents();
    EXPECT_EQ(1.0, counter->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectControl_ByFunction) {
    // Test that connecting and disconnecting with a function value works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "var connection = engine.connectControl('[Test]', 'co', reaction);"
            "connection.trigger();"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectControl_ByFunctionAllowDuplicateConnections) {
    // Test that duplicate connections are allowed when passing callbacks as functions.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "engine.connectControl('[Test]', 'co', reaction);"
            "engine.connectControl('[Test]', 'co', reaction);"
            // engine.trigger() has no way to know which connection to a ControlObject
            // to trigger, so it should trigger all of them.
            "engine.trigger('[Test]', 'co');"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    // The counter should have been incremented exactly twice.
    EXPECT_DOUBLE_EQ(2.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectControl_toDisconnectRemovesAllConnections) {
    // Test that every connection to a ControlObject is disconnected
    // by calling engine.connectControl(..., true). Individual connections
    // can only be disconnected by storing the connection object returned by
    // engine.connectControl and calling that object's 'disconnect' method.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "engine.connectControl('[Test]', 'co', reaction);"
            "engine.connectControl('[Test]', 'co', reaction);"
            "engine.trigger('[Test]', 'co');"
            "function disconnect() { "
            "  engine.connectControl('[Test]', 'co', reaction, 1);"
            "  engine.trigger('[Test]', 'co'); }"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_TRUE(evaluateAndAssert("disconnect()"));
    processEvents();
    // The counter should have been incremented exactly twice.
    EXPECT_DOUBLE_EQ(2.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectControl_ByLambda) {
    // Test that connecting with an anonymous function works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var connection = engine.connectControl('[Test]', 'co', function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); });"
            "connection.trigger();"
            "function disconnect() { "
            "  connection.disconnect();"
            "  engine.trigger('[Test]', 'co'); }"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_TRUE(evaluateAndAssert("disconnect()"));
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectionObject_Disconnect) {
    // Test that disconnecting using the 'disconnect' method on the connection
    // object returned from connectControl works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "var connection = engine.makeConnection('[Test]', 'co', reaction);"
            "connection.trigger();"
            "function disconnect() { "
            "  connection.disconnect();"
            "  engine.trigger('[Test]', 'co'); }"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_TRUE(evaluateAndAssert("disconnect()"));
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectionObject_reflectDisconnect) {
    // Test that checks if disconnecting yields the appropriate feedback
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(success) { "
            "  if (success) {"
            "    var pass = engine.getValue('[Test]', 'passed');"
            "    engine.setValue('[Test]', 'passed', pass + 1.0); "
            "  }"
            "};"
            "var dummy_callback = function(value) {};"
            "var connection = engine.makeConnection('[Test]', 'co', dummy_callback);"
            "reaction(connection);"
            "reaction(connection.isConnected);"
            "var successful_disconnect = connection.disconnect();"
            "reaction(successful_disconnect);"
            "reaction(!connection.isConnected);"));
    processEvents();
    EXPECT_DOUBLE_EQ(4.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectionObject_DisconnectByPassingToConnectControl) {
    // Test that passing a connection object back to engine.connectControl
    // removes the connection
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));
    // The connections should be removed from the ControlObject which they were
    // actually connected to, regardless of the group and item arguments passed
    // to engine.connectControl() to remove the connection. All that should matter
    // is that a valid ControlObject is specified.
    auto dummy = std::make_unique<ControlObject>(ConfigKey("[Test]", "dummy"));

    EXPECT_TRUE(evaluateAndAssert(
            "var reaction = function(value) { "
            "  var pass = engine.getValue('[Test]', 'passed');"
            "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
            "var connection1 = engine.connectControl('[Test]', 'co', reaction);"
            "var connection2 = engine.connectControl('[Test]', 'co', reaction);"
            "function disconnectConnection1() { "
            "  engine.connectControl('[Test]',"
            "                        'dummy',"
            "                        connection1);"
            "  engine.trigger('[Test]', 'co'); }"
            // Whether a 4th argument is passed to engine.connectControl does not matter.
            "function disconnectConnection2() { "
            "  engine.connectControl('[Test]',"
            "                        'dummy',"
            "                        connection2, true);"
            "  engine.trigger('[Test]', 'co'); }"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_TRUE(evaluateAndAssert("disconnectConnection1()"));
    processEvents();
    // The counter should have been incremented once by connection2.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
    EXPECT_TRUE(evaluateAndAssert("disconnectConnection2()"));
    processEvents();
    // The counter should not have changed.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectionObject_MakesIndependentConnection) {
    // Test that multiple connections can be made to the same CO with
    // the same callback function and that calling their 'disconnect' method
    // only disconnects the callback for that object.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    EXPECT_TRUE(evaluateAndAssert(
            "var incrementCounterCO = function () {"
            "  var counter = engine.getValue('[Test]', 'counter');"
            "  engine.setValue('[Test]', 'counter', counter + 1);"
            "};"
            "var connection1 = engine.makeConnection('[Test]', 'co', incrementCounterCO);"
            // Make a second connection with the same ControlObject
            // to check that disconnecting one does not disconnect both.
            "var connection2 = engine.makeConnection('[Test]', 'co', incrementCounterCO);"
            "function changeTestCoValue() {"
            "  var testCoValue = engine.getValue('[Test]', 'co');"
            "  engine.setValue('[Test]', 'co', testCoValue + 1);"
            "}"
            "function disconnectConnection1() {"
            "  connection1.disconnect();"
            "}"));
    EXPECT_TRUE(evaluateAndAssert("changeTestCoValue()"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_EQ(2.0, counter->get());

    EXPECT_TRUE(evaluateAndAssert("disconnectConnection1()"));
    // Only the callback for connection1 should have disconnected;
    // the callback for connection2 should still be connected, so
    // changing the CO they were both connected to should
    // increment the counter once.
    EXPECT_TRUE(evaluateAndAssert("changeTestCoValue()"));
    processEvents();
    EXPECT_EQ(3.0, counter->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectionObject_trigger) {
    // Test that triggering using the 'trigger' method on the connection
    // object returned from connectControl works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    EXPECT_TRUE(evaluateAndAssert(
            "var incrementCounterCO = function () {"
            "  var counter = engine.getValue('[Test]', 'counter');"
            "  engine.setValue('[Test]', 'counter', counter + 1);"
            "};"
            "var connection1 = engine.makeConnection('[Test]', 'co', incrementCounterCO);"
            // Make a second connection with the same ControlObject
            // to check that triggering a connection object only triggers that callback,
            // not every callback connected to its ControlObject.
            "var connection2 = engine.makeConnection('[Test]', 'co', incrementCounterCO);"
            "connection1.trigger();"));
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, counter->get());
}

TEST_F(ControllerScriptEngineLegacyTest, connectionExecutesWithCorrectThisObject) {
    // Test that callback functions are executed with JavaScript's
    // 'this' keyword referring to the object in which the connection
    // was created.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    EXPECT_TRUE(evaluateAndAssert(
            "var TestObject = function () {"
            "  this.executeTheCallback = true;"
            "  this.connection = engine.makeConnection('[Test]', 'co', function () {"
            "    if (this.executeTheCallback) {"
            "      engine.setValue('[Test]', 'passed', 1);"
            "    }"
            "  }.bind(this));"
            "};"
            "var someObject = new TestObject();"
            "someObject.connection.trigger();"));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

template<int N>
QByteArray intByteArray(const char (&array)[N]) {
    return QByteArray(array, N);
}

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetAllWellKnownCharsets) {
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    QMetaEnum charsetEnumEntry = QMetaEnum::fromType<
            ControllerScriptInterfaceLegacy::WellKnownCharsets>();

    QString script = R"(
        var workingCharsetCounter = 0;
        var failedCharsets = [];
        var expectedLengths = {
            "UCS2": 68, "ISO_10646_UCS_2": 68, "UTF_16": 66, "UTF_16BE": 66, "UTF_16LE": 66,
            "UTF_32": 128, "UTF_32BE": 128, "UTF_32LE": 128, "ISO_2022_KR": 54, "ISO_2022_Locale_KO_Version_1": 54,
            "HZ_GB_2312": 49, "Latin1": 33, "ISO_8859_1": 33, "Latin9": 32, "ISO_8859_15": 32,
            "UTF_8": 63, "UTF_7": 70, "SCSU": 51, "BOCU_1": 53, "CESU_8": 65, "US_ASCII": 32,
            "GB18030": 69, "ISO_8859_2": 32, "ISO_8859_3": 32, "ISO_8859_4": 32, "ISO_8859_5": 32,
            "ISO_8859_6": 32, "ISO_8859_7": 32, "ISO_8859_8": 32, "ISO_8859_9": 32, "ISO_8859_10": 32,
            "ISO_8859_13": 32, "ISO_8859_14": 32, "Shift_JIS": 39, "EUC_JP": 39, "Big5": 34,
            "Big5_HKSCS": 39, "GBK": 39, "GB2312": 39, "EUC_KR": 44, "CP1363": 44, "KSC_5601": 44,
            "Windows_874_2000": 32, "TIS_620": 32, "IBM437": 32, "IBM775": 32, "IBM850": 32,
            "CP851": 32, "IBM852": 32, "IBM855": 32, "IBM857": 32, "IBM00858": 32, "IBM860": 32,
            "IBM861": 32, "IBM862": 32, "IBM863": 32, "IBM864": 32, "IBM865": 32, "IBM866": 32,
            "IBM868": 32, "IBM869": 32, "KOI8_R": 32, "KOI8_U": 32, "Windows_1250": 32, "Windows_1251": 32,
            "Windows_1252": 32, "Windows_1253": 32, "Windows_1254": 32, "Windows_1255": 32, "Windows_1256": 32,
            "Windows_1257": 32, "Windows_1258": 32, "Macintosh": 32, "X_Mac_Greek": 32, "X_Mac_Cyrillic": 32,
            "X_Mac_CentralEuroRoman": 32, "X_Mac_Turkish": 32, "HP_Roman8": 32, "Adobe_Standard_Encoding": 32,
            "ISO_2022_JP": 51, "ISO_2022_JP_1": 51, "ISO_2022_JP_2": 63, "ISO_2022_CN": 47, "ISO_2022_CN_EXT": 47,
            "IBM037": 32, "IBM273": 32, "IBM277": 32, "IBM278": 32, "IBM280": 32, "IBM284": 32,
            "IBM285": 32, "IBM290": 32, "IBM297": 32, "IBM420": 32, "IBM424": 32, "IBM500": 32,
            "IBM_Thai": 32, "IBM870": 32, "IBM871": 32, "IBM918": 32, "IBM1026": 32, "IBM1047": 32,
            "IBM01140": 32, "IBM01141": 32, "IBM01142": 32, "IBM01143": 32, "IBM01144": 32, "IBM01145": 32,
            "IBM01146": 32, "IBM01147": 32, "IBM01148": 32, "IBM01149": 32
        };
    )";

    for (int i = 0; i < charsetEnumEntry.keyCount(); ++i) {
        QString key = QString::fromUtf8(charsetEnumEntry.key(i));
        script += QString(R"(
            var charset = engine.WellKnownCharsets.%1;
            var result = engine.convertCharset(charset, 'Hello, 世界! שלום! こんにちは! 안녕하세요! 😊');
            var expectedLength = expectedLengths['%1'];
            var resultLength = result.byteLength;
            if (resultLength === expectedLength) {
                workingCharsetCounter++;
            } else {
                failedCharsets.push(`Charset: %1  Result: ${result}  Result-Length: ${resultLength}  Expected-Length: ${expectedLength}`);
            }
        )")
                          .arg(key);
    }

    script += R"(
        engine.setValue('[Test]', 'counter', workingCharsetCounter);
        failedCharsets.join('\n');
    )";

    QJSValue result = evaluate(script);
    EXPECT_FALSE(result.isError());

    QString failedCharsetsResult = result.toString();
    if (!failedCharsetsResult.isEmpty()) {
        ADD_FAILURE() << "Charsets with result length not equal to expected:\n"
                      << failedCharsetsResult.toStdString();
    }

    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    processEvents();
    EXPECT_DOUBLE_EQ(counter->get(), charsetEnumEntry.keyCount());
}

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetCorrectValueWellKnown) {
    const auto result = evaluate(
            "engine.convertCharset(engine.WellKnownCharsets.Latin9, 'Hello!')");

    // ISO-8859-15 ecoded 'Hello!'
    EXPECT_EQ(qjsvalue_cast<QByteArray>(result),
            intByteArray({0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x21}));
}

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetUnsupportedChars) {
    auto result = qjsvalue_cast<QByteArray>(
            evaluate("engine.convertCharset(engine.WellKnownCharsets.ISO_8859_15, 'مايأ نامز')"));

    EXPECT_EQ(result,
            intByteArray(
                    {0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00}));
}

#ifdef MIXXX_USE_QML
class MockScreenRender : public ControllerRenderingEngine {
  public:
    MockScreenRender(const LegacyControllerMapping::ScreenInfo& info)
            : ControllerRenderingEngine(info, new ControllerEngineThreadControl){};
    MOCK_METHOD(void,
            requestSendingFrameData,
            (Controller * controller, const QByteArray& frame),
            (override));
};

TEST_F(ControllerScriptEngineLegacyTest, screenWontSentRawDataIfNotConfigured) {
    SETUP_LOG_CAPTURE();
    LegacyControllerMapping::ScreenInfo dummyScreen{
            "",                                                    // identifier
            QSize(0, 0),                                           // size
            10,                                                    // target_fps
            1,                                                     // msaa
            std::chrono::milliseconds(10),                         // splash_off
            QImage::Format_RGB16,                                  // pixelFormat
            LegacyControllerMapping::ScreenInfo::ColorEndian::Big, // endian
            false,                                                 // rawData
            false                                                  // reversedColor
    };
    QImage dummyFrame;
    // Allocate screen on the heap as it need to outlive the this function,
    // since the engine will take ownership of it
    std::shared_ptr<MockScreenRender> pDummyRender =
            std::make_shared<MockScreenRender>(dummyScreen);
    EXPECT_CALL(*pDummyRender, requestSendingFrameData(_, _)).Times(0);
    EXPECT_LOG_MSG(QtWarningMsg,
            "Could not find a valid transform function but the screen doesn't "
            "accept raw data. Aborting screen rendering.");

    renderingScreens().insert(dummyScreen.identifier, pDummyRender);
    rootItems().emplace(dummyScreen.identifier,
            std::make_unique<mixxx::qml::QmlMixxxControllerScreen>());

    testHandleScreen(
            dummyScreen,
            dummyFrame,
            QDateTime::currentDateTime());

    ASSERT_ALL_EXPECTED_MSG();
}

TEST_F(ControllerScriptEngineLegacyTest, screenWillSentRawDataIfConfigured) {
    SETUP_LOG_CAPTURE();
    LegacyControllerMapping::ScreenInfo dummyScreen{
            "",                                                    // identifier
            QSize(0, 0),                                           // size
            10,                                                    // target_fps
            1,                                                     // msaa
            std::chrono::milliseconds(10),                         // splash_off
            QImage::Format_RGB16,                                  // pixelFormat
            LegacyControllerMapping::ScreenInfo::ColorEndian::Big, // endian
            false,                                                 // reversedColor
            true                                                   // rawData
    };
    QImage dummyFrame;
    // Allocate screen on the heap as it need to outlive the this function,
    // since the engine will take ownership of it
    std::shared_ptr<MockScreenRender> pDummyRender =
            std::make_shared<MockScreenRender>(dummyScreen);
    EXPECT_CALL(*pDummyRender, requestSendingFrameData(_, QByteArray()));

    renderingScreens().insert(dummyScreen.identifier, pDummyRender);
    rootItems().emplace(dummyScreen.identifier,
            std::make_unique<mixxx::qml::QmlMixxxControllerScreen>());

    testHandleScreen(
            dummyScreen,
            dummyFrame,
            QDateTime::currentDateTime());

    ASSERT_ALL_EXPECTED_MSG();
}
#endif
