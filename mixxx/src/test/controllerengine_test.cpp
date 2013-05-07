
#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QFile>
#include <QThread>

#include "controlobject.h"
#include "controlpotmeter.h"
#include "configobject.h"
#include "controllers/controllerengine.h"
#include "test/mixxxtest.h"

namespace {

class ControllerEngineTest : public MixxxTest {
  protected:
    virtual void SetUp() {
        qDebug() << "SetUp";
        static int argc = 1;
        static char* argv[2] = { "test", NULL };
        QThread::currentThread()->setObjectName("Main");
        app = new QApplication(argc, argv);
        new ControlPotmeter(ConfigKey("[Test]", "potmeter"),-1.,1.);
        Controller* pController = NULL;
        cEngine = new ControllerEngine(pController);
        cEngine->setDebug(true);
        cEngine->setPopups(false);
    }

    virtual void TearDown() {
        qDebug() << "TearDown";
        cEngine->gracefulShutdown();
        delete cEngine;
        delete app;
    }

    QApplication *app;
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
    cEngine->execute("setValue");
    ControlObject::sync();
    EXPECT_DOUBLE_EQ(co->get(), 1.0f);

    delete co;
}

TEST_F(ControllerEngineTest, scriptGetSetValue) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "getSetValue = function() { var val = engine.getValue('[Channel1]', 'co'); engine.setValue('[Channel1]', 'co', val + 1); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    ScopedControl co(new ControlObject(ConfigKey("[Channel1]", "co")));
    co->set(0.0);
    cEngine->execute("getSetValue");
    ControlObject::sync();
    EXPECT_DOUBLE_EQ(co->get(), 1.0f);
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

    EXPECT_TRUE(cEngine->execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    app->processEvents();
    EXPECT_TRUE(cEngine->execute("checkConnectDisconnectControl"));
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

    EXPECT_TRUE(cEngine->execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    app->processEvents();
    EXPECT_TRUE(cEngine->execute("checkConnectDisconnectControl"));
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

    EXPECT_TRUE(cEngine->execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    app->processEvents();
    EXPECT_TRUE(cEngine->execute("checkConnectDisconnectControl"));
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

    EXPECT_TRUE(cEngine->execute("testConnectDisconnectControl"));
    EXPECT_TRUE(cEngine->execute("checkConnectDisconnectControl"));
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

    EXPECT_TRUE(cEngine->execute("testConnectDisconnectControl"));
    // trigger() calls are processed via QueuedConnection. Use processEvents()
    // to cause Qt to deliver them.
    app->processEvents();
    EXPECT_TRUE(cEngine->execute("checkConnectDisconnectControl"));
}

TEST_F(ControllerEngineTest, setInvalidControlObject) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "setValue = function() { engine.setValue('[Nothing]', 'nothing', 1.0); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(cEngine->execute("setValue"));
}

TEST_F(ControllerEngineTest, getInvalidControlObject) {
    ScopedTemporaryFile script(makeTemporaryFile(
        "getValue = function() { return engine.getValue('[Nothing]', 'nothing'); }\n"));

    cEngine->evaluate(script->fileName());
    EXPECT_FALSE(cEngine->hasErrors(script->fileName()));

    EXPECT_TRUE(cEngine->execute("getValue"));
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
    EXPECT_TRUE(cEngine->execute("setUp"));
    ControlObject::sync();

    // The actual test
    //  TODO: Have the JS call a function in this test class so the test framework
    //  can tell if it actually passed or not
    co->set(2.5);
    ControlObject::sync();
}

}
