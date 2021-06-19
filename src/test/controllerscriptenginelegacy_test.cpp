#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"

#include <QScopedPointer>
#include <QTemporaryFile>
#include <QThread>
#include <QtDebug>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/controllerdebug.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/time.h"

typedef std::unique_ptr<QTemporaryFile> ScopedTemporaryFile;

class ControllerScriptEngineLegacyTest : public MixxxTest {
  protected:
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
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        QThread::currentThread()->setObjectName("Main");
        cEngine = new ControllerScriptEngineLegacy(nullptr);
        cEngine->initialize();
        ControllerDebug::setTesting(true);
    }

    void TearDown() override {
        delete cEngine;
        mixxx::Time::setTestMode(false);
    }

    bool evaluateScriptFile(const QFileInfo& scriptFile) {
        return cEngine->evaluateScriptFile(scriptFile);
    }

    QJSValue evaluate(const QString& code) {
        return cEngine->jsEngine()->evaluate(code);
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

    ControllerScriptEngineLegacy* cEngine;
};

TEST_F(ControllerScriptEngineLegacyTest, commonScriptHasNoErrors) {
    QFileInfo commonScript("./res/controllers/common-controller-scripts.js");
    EXPECT_TRUE(evaluateScriptFile(commonScript));
}

TEST_F(ControllerScriptEngineLegacyTest, setValue) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Test]', 'co', 1.0);"));
    EXPECT_DOUBLE_EQ(1.0, co->get());
}

TEST_F(ControllerScriptEngineLegacyTest, getValue_InvalidKey) {
    ControllerDebug::setEnabled(false);
    ControllerDebug::setTesting(false);
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('', '');"));
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('', 'invalid');"));
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('[Invalid]', '');"));
    ControllerDebug::setTesting(true);
    ControllerDebug::setEnabled(true);
}

TEST_F(ControllerScriptEngineLegacyTest, setValue_InvalidControl) {
    ControllerDebug::setEnabled(false);
    ControllerDebug::setTesting(false);
    EXPECT_TRUE(evaluateAndAssert("engine.setValue('[Nothing]', 'nothing', 1.0);"));
    ControllerDebug::setTesting(true);
    ControllerDebug::setEnabled(true);
}

TEST_F(ControllerScriptEngineLegacyTest, getValue_InvalidControl) {
    ControllerDebug::setEnabled(false);
    ControllerDebug::setTesting(false);
    EXPECT_TRUE(evaluateAndAssert("engine.getValue('[Nothing]', 'nothing');"));
    ControllerDebug::setTesting(true);
    ControllerDebug::setEnabled(true);
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
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);

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

    // Advance time to 2x the threshold.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);

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
