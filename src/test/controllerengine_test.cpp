#include <QtDebug>
#include <QThread>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "preferences/usersettings.h"
#include "controllers/controllerengine.h"
#include "controllers/controllerdebug.h"
#include "controllers/softtakeover.h"
#include "test/mixxxtest.h"
#include "util/memory.h"
#include "util/time.h"

class ControllerEngineTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        QThread::currentThread()->setObjectName("Main");
        cEngine = new ControllerEngine(nullptr);
        pScriptEngine = cEngine->m_pEngine;
        ControllerDebug::enable();
        cEngine->setPopups(false);
    }

    void TearDown() override {
        cEngine->gracefulShutdown();
        delete cEngine;
        mixxx::Time::setTestMode(false);
    }

    bool execute(const QString& functionName) {
        QScriptValue function = cEngine->wrapFunctionCode(functionName, 0);
        return cEngine->internalExecute(QScriptValue(), function,
                                        QScriptValueList());
    }

    ControllerEngine *cEngine;
    QScriptEngine *pScriptEngine;
};

TEST_F(ControllerEngineTest, commonScriptHasNoErrors) {
    QString commonScript = "./res/controllers/common-controller-scripts.js";
    cEngine->evaluate(commonScript);
    EXPECT_FALSE(cEngine->hasErrors(commonScript));
}

TEST_F(ControllerEngineTest, setValue) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    EXPECT_TRUE(execute("function() { engine.setValue('[Test]', 'co', 1.0); }"));
    EXPECT_DOUBLE_EQ(1.0, co->get());
}

TEST_F(ControllerEngineTest, setValue_InvalidControl) {
    EXPECT_TRUE(execute("function() { engine.setValue('[Nothing]', 'nothing', 1.0); }"));
}

TEST_F(ControllerEngineTest, getValue_InvalidControl) {
    EXPECT_TRUE(execute("function() { return engine.getValue('[Nothing]', 'nothing'); }"));
}

TEST_F(ControllerEngineTest, setValue_IgnoresNaN) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    co->set(10.0);
    EXPECT_TRUE(execute("function() { engine.setValue('[Test]', 'co', NaN); }"));
    EXPECT_DOUBLE_EQ(10.0, co->get());
}


TEST_F(ControllerEngineTest, getSetValue) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    EXPECT_TRUE(execute("function() { engine.setValue('[Test]', 'co', engine.getValue('[Test]', 'co') + 1); }"));
    EXPECT_DOUBLE_EQ(1.0, co->get());
}

TEST_F(ControllerEngineTest, setParameter) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', 1.0); }"));
    EXPECT_DOUBLE_EQ(10.0, co->get());
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', 0.0); }"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', 0.5); }"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerEngineTest, setParameter_OutOfRange) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    EXPECT_TRUE(execute("function () { engine.setParameter('[Test]', 'co', 1000); }"));
    EXPECT_DOUBLE_EQ(10.0, co->get());
    EXPECT_TRUE(execute("function () { engine.setParameter('[Test]', 'co', -1000); }"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());
}

TEST_F(ControllerEngineTest, setParameter_NaN) {
    // Test that NaNs are ignored.
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', NaN); }"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerEngineTest, getSetParameter) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', "
                        "  engine.getParameter('[Test]', 'co') + 0.1); }"));
    EXPECT_DOUBLE_EQ(2.0, co->get());
}

TEST_F(ControllerEngineTest, softTakeover_setValue) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    co->setParameter(0.0);
    EXPECT_TRUE(execute("function() {"
                        "  engine.softTakeover('[Test]', 'co', true);"
                        "  engine.setValue('[Test]', 'co', 0.0); }"));
    // The first set after enabling is always ignored.
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Time elapsed is not greater than the threshold, so we do not ignore this
    // set.
    EXPECT_TRUE(execute("function() { engine.setValue('[Test]', 'co', -10.0); }"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Advance time to 2x the threshold.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Ignore the change since it occurred after the threshold and is too large.
    EXPECT_TRUE(execute("function() { engine.setValue('[Test]', 'co', -10.0); }"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerEngineTest, softTakeover_setParameter) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    co->setParameter(0.0);
    EXPECT_TRUE(execute("function() {"
                        "  engine.softTakeover('[Test]', 'co', true);"
                        "  engine.setParameter('[Test]', 'co', 1.0); }"));
    // The first set after enabling is always ignored.
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Time elapsed is not greater than the threshold, so we do not ignore this
    // set.
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', 0.0); }"));
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Advance time to 2x the threshold.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() * 2);

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    // Ignore the change since it occurred after the threshold and is too large.
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', 0.0); }"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerEngineTest, softTakeover_ignoreNextValue) {
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    co->setParameter(0.0);
    EXPECT_TRUE(execute("function() {"
                        "  engine.softTakeover('[Test]', 'co', true);"
                        "  engine.setParameter('[Test]', 'co', 1.0); }"));
    // The first set after enabling is always ignored.
    EXPECT_DOUBLE_EQ(-10.0, co->get());

    // Change the control internally (putting it out of sync with the
    // ControllerEngine).
    co->setParameter(0.5);

    EXPECT_TRUE(execute("function() { engine.softTakeoverIgnoreNextValue('[Test]', 'co'); }"));

    // We would normally allow this set since it is below the time threshold,
    // but we are ignoring the next value.
    EXPECT_TRUE(execute("function() { engine.setParameter('[Test]', 'co', 0.0); }"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerEngineTest, reset) {
    // Test that NaNs are ignored.
    auto co = std::make_unique<ControlPotmeter>(ConfigKey("[Test]", "co"),
                                                -10.0, 10.0);
    co->setParameter(1.0);
    EXPECT_TRUE(execute("function() { engine.reset('[Test]', 'co'); }"));
    EXPECT_DOUBLE_EQ(0.0, co->get());
}

TEST_F(ControllerEngineTest, log) {
    EXPECT_TRUE(execute("function() { engine.log('Test that logging works.'); }"));
}

TEST_F(ControllerEngineTest, trigger) {
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "var connection = engine.connectControl('[Test]', 'co', reaction);"
        "engine.trigger('[Test]', 'co');"));
    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

// ControllerEngine::connectControl has a lot of quirky, inconsistent legacy behaviors
// depending on how it is invoked, so we need a lot of tests to make sure old scripts
// do not break.

TEST_F(ControllerEngineTest, connectControl_ByString) {
    // Test that connecting and disconnecting by function name works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "engine.connectControl('[Test]', 'co', 'reaction');"
        "engine.trigger('[Test]', 'co');"
        "function disconnect() { "
        "  engine.connectControl('[Test]', 'co', 'reaction', 1);"
        "  engine.trigger('[Test]', 'co'); }"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("disconnect"));
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerEngineTest, connectControl_ByStringForbidDuplicateConnections) {
    // Test that connecting a control to a callback specified by a string
    // does not make duplicate connections. This behavior is inconsistent
    // with the behavior when specifying a callback as a function, but
    // this is how it has been done, so keep the behavior to ensure old scripts
    // do not break.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "engine.connectControl('[Test]', 'co', 'reaction');"
        "engine.connectControl('[Test]', 'co', 'reaction');"
        "engine.trigger('[Test]', 'co');"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerEngineTest,
       connectControl_ByStringRedundantConnectionObjectsAreNotIndependent) {
    // Test that multiple connections are not allowed when passing
    // the callback to engine.connectControl as a function name string.
    // This is weird and inconsistent, but it is how it has been done,
    // so keep this behavior to make sure old scripts do not break.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    ScopedTemporaryFile script(makeTemporaryFile(
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
        "};"
    ));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    execute("changeTestCoValue");
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_EQ(1.0, counter->get());

    execute("disconnectConnection2");
    // The connection objects should refer to the same connection,
    // so disconnecting one should disconnect both.
    execute("changeTestCoValue");
    application()->processEvents();
    EXPECT_EQ(1.0, counter->get());
}

TEST_F(ControllerEngineTest, connectControl_ByFunction) {
    // Test that connecting and disconnecting with a function value works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "var connection = engine.connectControl('[Test]', 'co', reaction);"
        "connection.trigger();"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerEngineTest, connectControl_ByFunctionAllowDuplicateConnections) {
    // Test that duplicate connections are allowed when passing callbacks as functions.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "engine.connectControl('[Test]', 'co', reaction);"
        "engine.connectControl('[Test]', 'co', reaction);"
        // engine.trigger() has no way to know which connection to a ControlObject
        // to trigger, so it should trigger all of them.
        "engine.trigger('[Test]', 'co');"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    // The counter should have been incremented exactly twice.
    EXPECT_DOUBLE_EQ(2.0, pass->get());
}

TEST_F(ControllerEngineTest, connectControl_toDisconnectRemovesAllConnections) {
    // Test that every connection to a ControlObject is disconnected
    // by calling engine.connectControl(..., true). Individual connections
    // can only be disconnected by storing the connection object returned by
    // engine.connectControl and calling that object's 'disconnect' method.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "engine.connectControl('[Test]', 'co', reaction);"
        "engine.connectControl('[Test]', 'co', reaction);"
        "engine.trigger('[Test]', 'co');"
        "function disconnect() { "
        "  engine.connectControl('[Test]', 'co', reaction, 1);"
        "  engine.trigger('[Test]', 'co'); }"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("disconnect"));
    application()->processEvents();
    // The counter should have been incremented exactly twice.
    EXPECT_DOUBLE_EQ(2.0, pass->get());
}

TEST_F(ControllerEngineTest, connectControl_ByLambda) {
    // Test that connecting with an anonymous function works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var connection = engine.connectControl('[Test]', 'co', function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); });"
        "connection.trigger();"
        "function disconnect() { "
        "  connection.disconnect();"
        "  engine.trigger('[Test]', 'co'); }"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("disconnect"));
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerEngineTest, connectionObject_Disconnect) {
    // Test that disconnecting using the 'disconnect' method on the connection
    // object returned from connectControl works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var reaction = function(value) { "
        "  var pass = engine.getValue('[Test]', 'passed');"
        "  engine.setValue('[Test]', 'passed', pass + 1.0); };"
        "var connection = engine.makeConnection('[Test]', 'co', reaction);"
        "connection.trigger();"
        "function disconnect() { "
        "  connection.disconnect();"
        "  engine.trigger('[Test]', 'co'); }"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("disconnect"));
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}
TEST_F(ControllerEngineTest, connectionObject_reflectDisconnect) {
    // Test that checks if disconnecting yields the appropriate feedback
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
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
        "reaction(!connection.isConnected);"
    ));
    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    application()->processEvents();
    EXPECT_DOUBLE_EQ(4.0, pass->get());
}


TEST_F(ControllerEngineTest, connectionObject_DisconnectByPassingToConnectControl) {
    // Test that passing a connection object back to engine.connectControl
    // removes the connection
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));
    // The connections should be removed from the ControlObject which they were
    // actually connected to, regardless of the group and item arguments passed
    // to engine.connectControl() to remove the connection. All that should matter
    // is that a valid ControlObject is specified.
    auto dummy = std::make_unique<ControlObject>(ConfigKey("[Test]", "dummy"));

    ScopedTemporaryFile script(makeTemporaryFile(
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

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("disconnectConnection1"));
    application()->processEvents();
    // The counter should have been incremented once by connection2.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
    EXPECT_TRUE(execute("disconnectConnection2"));
    application()->processEvents();
    // The counter should not have changed.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerEngineTest, connectionObject_MakesIndependentConnection) {
    // Test that multiple connections can be made to the same CO with
    // the same callback function and that calling their 'disconnect' method
    // only disconnects the callback for that object.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    ScopedTemporaryFile script(makeTemporaryFile(
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
        "}"
    ));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    execute("changeTestCoValue");
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_EQ(2.0, counter->get());

    execute("disconnectConnection1");
    // Only the callback for connection1 should have disconnected;
    // the callback for connection2 should still be connected, so
    // changing the CO they were both connected to should
    // increment the counter once.
    execute("changeTestCoValue");
    application()->processEvents();
    EXPECT_EQ(3.0, counter->get());
}

TEST_F(ControllerEngineTest, connectionObject_trigger) {
    // Test that triggering using the 'trigger' method on the connection
    // object returned from connectControl works.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto counter = std::make_unique<ControlObject>(ConfigKey("[Test]", "counter"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var incrementCounterCO = function () {"
        "  var counter = engine.getValue('[Test]', 'counter');"
        "  engine.setValue('[Test]', 'counter', counter + 1);"
        "};"
        "var connection1 = engine.makeConnection('[Test]', 'co', incrementCounterCO);"
        // Make a second connection with the same ControlObject
        // to check that triggering a connection object only triggers that callback,
        // not every callback connected to its ControlObject.
        "var connection2 = engine.makeConnection('[Test]', 'co', incrementCounterCO);"
        "connection1.trigger();"
    ));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, counter->get());
}


TEST_F(ControllerEngineTest, connectionExecutesWithCorrectThisObject) {
    // Test that callback functions are executed with JavaScript's
    // 'this' keyword referring to the object in which the connection
    // was created.
    auto co = std::make_unique<ControlObject>(ConfigKey("[Test]", "co"));
    auto pass = std::make_unique<ControlObject>(ConfigKey("[Test]", "passed"));

    ScopedTemporaryFile script(makeTemporaryFile(
        "var TestObject = function () {"
        "  this.executeTheCallback = true;"
        "  this.connection = engine.makeConnection('[Test]', 'co', function () {"
        "    if (this.executeTheCallback) {"
        "      engine.setValue('[Test]', 'passed', 1);"
        "    }"
        "  });"
        "};"
        "var someObject = new TestObject();"
        "someObject.connection.trigger();"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));
    // ControlObjectScript connections are processed via QueuedConnection. Use
    // processEvents() to cause Qt to deliver them.
    application()->processEvents();
    // The counter should have been incremented exactly once.
    EXPECT_DOUBLE_EQ(1.0, pass->get());
}

TEST_F(ControllerEngineTest, colorProxy) {
    QList<PredefinedColorPointer> allColors = Color::kPredefinedColorsSet.allColors;
    for (int i = 0; i < allColors.length(); ++i) {
        PredefinedColorPointer color = allColors[i];
        qDebug() << "Testing color " << color->m_sName;
        QScriptValue jsColor = pScriptEngine->evaluate("color.predefinedColorFromId(" + QString::number(color->m_iId) + ")");
        EXPECT_EQ(jsColor.property("red").toInt32(), color->m_defaultRgba.red());
        EXPECT_EQ(jsColor.property("green").toInt32(), color->m_defaultRgba.green());
        EXPECT_EQ(jsColor.property("blue").toInt32(), color->m_defaultRgba.blue());
        EXPECT_EQ(jsColor.property("alpha").toInt32(), color->m_defaultRgba.alpha());
        EXPECT_EQ(jsColor.property("id").toInt32(), color->m_iId);

        QScriptValue jsColor2 = pScriptEngine->evaluate("color.predefinedColorsList()["
                                                        + QString::number(i) + "]");
        EXPECT_EQ(jsColor2.property("red").toInt32(), color->m_defaultRgba.red());
        EXPECT_EQ(jsColor2.property("green").toInt32(), color->m_defaultRgba.green());
        EXPECT_EQ(jsColor2.property("blue").toInt32(), color->m_defaultRgba.blue());
        EXPECT_EQ(jsColor2.property("alpha").toInt32(), color->m_defaultRgba.alpha());
        EXPECT_EQ(jsColor2.property("id").toInt32(), color->m_iId);
    }
}
