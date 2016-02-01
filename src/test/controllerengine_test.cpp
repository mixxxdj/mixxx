
#include <gtest/gtest.h>
#include <QtDebug>
#include <QObject>
#include <QFile>
#include <QThread>

#include "controlobject.h"
#include "controlpotmeter.h"
#include "preferences/usersettings.h"
#include "controllers/controllerengine.h"
#include "controllers/controllerdebug.h"
#include "test/mixxxtest.h"

class ControllerEngineTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        qDebug() << "SetUp";
        QThread::currentThread()->setObjectName("Main");
        new ControlPotmeter(ConfigKey("[Test]", "potmeter"),-1.,1.);
        cEngine = new ControllerEngine(nullptr);
        ControllerDebug::setEnabled(true);
        cEngine->setPopups(false);
    }

    virtual void TearDown() {
        qDebug() << "TearDown";
        cEngine->gracefulShutdown();
        delete cEngine;
    }

    bool execute(const QString& functionName) {
        QScriptValue function = cEngine->resolveFunction(functionName);
        return cEngine->internalExecute(QScriptValue(), function,
                                        QScriptValueList());
    }

    ControllerEngine *cEngine;
};

TEST_F(ControllerEngineTest, commonScriptHasNoErrors) {
    QString commonScript = "./res/controllers/common-controller-scripts.js";
    cEngine->evaluate(commonScript);
    EXPECT_FALSE(cEngine->hasErrors(commonScript));
}

TEST_F(ControllerEngineTest, scriptSetValue) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "setValue = function() { engine.setValue('[Channel1]', 'co', 1.0); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    ControlObject *co = new ControlObject(ConfigKey("[Channel1]", "co"));
    co->set(0.0);
    execute("setValue");
    EXPECT_DOUBLE_EQ(co->get(), 1.0);

    delete co;
}

TEST_F(ControllerEngineTest, scriptGetSetValue) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "getSetValue = function() { var val = engine.getValue('[Channel1]', 'co'); engine.setValue('[Channel1]', 'co', val + 1); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    ScopedControl co(new ControlObject(ConfigKey("[Channel1]", "co")));
    co->set(0.0);
    execute("getSetValue");
    EXPECT_DOUBLE_EQ(co->get(), 1.0);
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlNamedFunction) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "var executed = false;\n"
        "var connection;\n"
        "testConnectDisconnectControlCallback = function() {\n"
        "    executed = true;\n"
        "};\n"
        "testConnectDisconnectControl = function() { \n"
        "    connection = engine.connectControl('[Test]', 'potmeter', \n"
        "                            'testConnectDisconnectControlCallback');\n"
        "    engine.trigger('[Test]', 'potmeter');\n"
        "    return true;\n"
        "};\n"
        "checkConnectDisconnectControl = function() {\n"
        "    connection.disconnect();\n"
        "    if (!executed) {\n"
        "        throw 'Did Not Execute Callback';\n"
        "    }\n"
        "    return executed;\n"
        "};\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("checkConnectDisconnectControl"));
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlClosure) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "var executed = false;\n"
        "var connection;\n"
        "testConnectDisconnectControl = function() { \n"
        "    connection = engine.connectControl('[Test]', 'potmeter', \n"
        "        function() { executed = true; }\n"
        "    );\n"
        "    engine.trigger('[Test]', 'potmeter');\n"
        "    return true;\n"
        "};\n"
        "checkConnectDisconnectControl = function() {\n"
        "    connection.disconnect();\n"
        "    if (!executed) {\n"
        "        throw 'Did Not Execute Callback';\n"
        "    }\n"
        "    return executed;\n"
        "};\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("checkConnectDisconnectControl"));
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlIsDisconnected) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "var executed = false;\n"
        "var connection;\n"
        "testConnectDisconnectControl = function() { \n"
        "    connection = engine.connectControl('[Test]', 'potmeter', \n"
        "        function() { executed = true; }\n"
        "    );\n"
        "    if (typeof connection == 'undefined')\n"
        "        throw 'Unable to Connect controller';\n"
        "    connection.disconnect();\n"
        "    engine.trigger('[Test]', 'potmeter');\n"
        "    return true;\n"
        "};\n"
        "checkConnectDisconnectControl = function() {\n"
        "    if (executed) {\n"
        "        throw 'Callback was executed';\n"
        "    }\n"
        "    return executed==false;\n"
        "};\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("checkConnectDisconnectControl"));
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlIsDisconnectedByName) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "var executed = false;\n"
        "var connection;\n"
        "connectionCallback = function() { executed = true; }\n"
        "testConnectDisconnectControl = function() { \n"
        "    connection = engine.connectControl('[Test]', 'potmeter', 'connectionCallback');\n"
        "    if (typeof connection == 'undefined')\n"
        "        throw 'Unable to Connect controller';\n"
        "    engine.connectControl('[Test]', 'potmeter', 'connectionCallback', 1);\n"
        "    engine.trigger('[Test]', 'potmeter');\n"
        "    return true;\n"
        "};\n"
        "checkConnectDisconnectControl = function() {\n"
        "    if (executed) {\n"
        "        throw 'Callback was executed';\n"
        "    }\n"
        "    return executed==false;\n"
        "};\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("testConnectDisconnectControl"));
    EXPECT_TRUE(execute("checkConnectDisconnectControl"));
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlIsDisconnectedByObject) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "var executed = false;\n"
        "var connection;\n"
        "testConnectDisconnectControl = function() { \n"
        "    connection = engine.connectControl('[Test]', 'potmeter', \n"
        "        function() { executed = true; }\n"
        "    );\n"
        "    if (typeof connection == 'undefined')\n"
        "        throw 'Unable to Connect controller';\n"
        "    engine.connectControl('[Test]', 'potmeter', connection, 1);\n"
        "    engine.trigger('[Test]', 'potmeter');\n"
        "    return true;\n"
        "};\n"
        "checkConnectDisconnectControl = function() {\n"
        "    if (executed) {\n"
        "        throw 'Callback was executed';\n"
        "    }\n"
        "    return executed==false;\n"
        "};\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    application()->processEvents();
    EXPECT_TRUE(execute("checkConnectDisconnectControl"));
}

TEST_F(ControllerEngineTest, setInvalidControlObject) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "setValue = function() { engine.setValue('[Nothing]', 'nothing', 1.0); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("setValue"));
}

TEST_F(ControllerEngineTest, getInvalidControlObject) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "getValue = function() { return engine.getValue('[Nothing]', 'nothing'); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(execute("getValue"));
}

TEST_F(ControllerEngineTest, automaticReaction) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "setUp = function() { engine.connectControl('[Channel1]','co','reaction'); }\n"
        "reaction = function(value) { if (value == 2.5) print('TEST PASSED: '+value);\n"
        "else print('TEST FAILED!  TEST FAILED!  TEST FAILED: '+value);  "
        "return value; }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    ScopedControl co(new ControlObject(ConfigKey("[Channel1]", "co")));
    co->set(0.0);
    EXPECT_TRUE(execute("setUp"));

    // The actual test
    //  TODO: Have the JS call a function in this test class so the test framework
    //  can tell if it actually passed or not
    co->set(2.5);
}
