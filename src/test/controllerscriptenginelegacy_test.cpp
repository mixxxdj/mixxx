#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QByteArrayView>
#include <QMetaEnum>
#include <QScopedPointer>
#include <QTemporaryFile>
#include <QThread>
#include <QtDebug>
#include <bit>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#ifdef MIXXX_USE_QML
#include <QQuickItem>

#include "controllers/controllerenginethreadcontrol.h"
#include "controllers/rendering/controllerrenderingengine.h"
#endif
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0) // Latin9 is available form Qt 6.5
TEST_F(ControllerScriptEngineLegacyTest, convertCharsetCorrectValueStringCharset) {
    const auto result = evaluate(
            "engine.convertCharset(engine.Charset.Latin9, 'Hello! €')");

    EXPECT_EQ(qjsvalue_cast<QByteArray>(result),
            QByteArrayView::fromArray({'\x48',
                    '\x65',
                    '\x6c',
                    '\x6c',
                    '\x6f',
                    '\x21',
                    '\x20',
                    '\xA4'}));
}

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetUnsupportedChars) {
    auto result = qjsvalue_cast<QByteArray>(
            evaluate("engine.convertCharset(engine.Charset.Latin9, 'مايأ نامز ™')"));
    char sub = '\x1A'; // ASCII/Latin9 SUB character
    EXPECT_EQ(result,
            QByteArrayView::fromArray(
                    {sub, sub, sub, sub, '\x20', sub, sub, sub, sub, '\x20', sub}));
}
#endif

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetLatin1Eur) {
    const auto result = evaluate(
            "engine.convertCharset(engine.Charset.Latin1, 'Hello! ¤€')");

    char sub = '?'; // used by Qt for substitution
    EXPECT_EQ(qjsvalue_cast<QByteArray>(result),
            QByteArrayView::fromArray({'\x48',
                    '\x65',
                    '\x6c',
                    '\x6c',
                    '\x6f',
                    '\x21',
                    '\x20',
                    '\xA4',
                    sub}));
}

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetMultiByteEncoding) {
    auto result = qjsvalue_cast<QByteArray>(
            evaluate("engine.convertCharset(engine.Charset.UTF_16LE, 'مايأ نامز')"));
    EXPECT_EQ(result,
            QByteArrayView::fromArray({'\x45',
                    '\x06',
                    '\x27',
                    '\x06',
                    '\x4A',
                    '\x06',
                    '\x23',
                    '\x06',
                    '\x20',
                    '\x00',
                    '\x46',
                    '\x06',
                    '\x27',
                    '\x06',
                    '\x45',
                    '\x06',
                    '\x32',
                    '\x06'}));
}

#define COMPLICATEDSTRINGLITERAL "Hello, 世界! שלום! こんにちは! 안녕하세요! ™ 😊"

static int convertedCharsetForString(ControllerScriptInterfaceLegacy::Charset charset) {
    // the expected length after conversion of COMPLICATEDSTRINGLITERAL
    using enum ControllerScriptInterfaceLegacy::Charset;

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    switch (charset) {
    case UTF_8:
        return 67;
    case UTF_16LE:
    case UTF_16BE:
        return 70;
    case UTF_32LE:
    case UTF_32BE:
        return 136;
    case ASCII:
    case CentralEurope:
    case Cyrillic:
    case WesternEurope:
    case Greek:
    case Turkish:
    case Hebrew:
    case Arabic:
    case Baltic:
    case Vietnamese:
    case Latin9:
    case KOI8_U:
        return 34;
    case Latin1:
        // Latin1 is handled by Qt internally and 😊 becomes "??"
        return 35;
    case EUC_JP:
        return 53;
    case Shift_JIS:
    case EUC_KR:
    case Big5_HKSCS:
        return 52;
    case UCS2:
        return 72;
    case SCSU:
        return 55;
    case BOCU_1:
        return 56;
    case CESU_8:
        return 69;
    }
#else
    // Qt < 6.4 only supports these conversions
    switch (charset) {
    case UTF_8:
        return 67;
    case UTF_16LE:
    case UTF_16BE:
        return 70;
    case UTF_32LE:
    case UTF_32BE:
        return 136;
    case Latin1:
        return 35;
    default:
        return 0;
    }
#endif

    // unreachable, but gtest does not offer a way to assert this here.
    // returning 0 will almost certainly also result in a failure.
    return 0;
}

TEST_F(ControllerScriptEngineLegacyTest, convertCharsetAllCharset) {
    QMetaEnum charsetEnumEntry = QMetaEnum::fromType<
            ControllerScriptInterfaceLegacy::Charset>();

    for (int i = 0; i < charsetEnumEntry.keyCount(); ++i) {
        QString key = charsetEnumEntry.key(i);
        auto enumValue =
                static_cast<ControllerScriptInterfaceLegacy::Charset>(
                        charsetEnumEntry.value(i));
        QString source = QStringLiteral(
                "engine.convertCharset(engine.Charset.%1, "
                "'" COMPLICATEDSTRINGLITERAL "')")
                                 .arg(key);
        auto result = qjsvalue_cast<QByteArray>(evaluate(source));
        EXPECT_EQ(result.size(), convertedCharsetForString(enumValue))
                << "Unexpected length of converted string for encoding: '"
                << key.toStdString() << "'";
    }
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
