
#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QFile>
#include <QThread>

#include "controlobject.h"
#include "controlpotmeter.h"
#include "configobject.h"
#include "controllers/controller.h"
#include "controllers/controllerengine.h"

namespace {

class ControllerEngineTest : public testing::Test {
  protected:
    static void SetUpTestCase() {
        QApplication *app;
        qDebug() << "SetUp";
        static int argc = 1;
        static char* argv[2] = { "test", NULL };
        QThread::currentThread()->setObjectName("Main");
        app = new QApplication(argc, argv);
        new ControlPotmeter(ConfigKey("[Test]", "potmeter"),-1.,1.);
    }

    virtual void SetUp() {
        Controller* pController = NULL;
        scriptEngine = new ControllerEngine(pController);
        scriptEngine->setDebug(false);
        scriptEngine->setPopups(false);
        scriptEngine->start();
        while(!scriptEngine->isReady()) { }
    }

    static void TearDownTestCase() {
    }

    virtual void TearDown() {
        qDebug() << "TearDown";
        scriptEngine->gracefulShutdown();
        scriptEngine->wait();
        delete scriptEngine;
    }

    ControllerEngine *scriptEngine;
};

TEST_F(ControllerEngineTest, commonScriptHasNoErrors) {
    // ConfigObject<ConfigValue> config("~/.mixxx/mixxx.cfg");
    // QString commonScript = config.getConfigPath() + "/" +
    //         "/midi/midi-mappings-scripts.js";
    QString commonScript = "./res/controllers/common-controller-scripts.js";
    scriptEngine->evaluate(commonScript);
    EXPECT_FALSE(scriptEngine->hasErrors(commonScript));
}

TEST_F(ControllerEngineTest, scriptSetValue) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("setValue = function() { engine.setValue('[Channel1]', 'co', 1.0); }\n");
    f.close();

    scriptEngine->evaluate(script);;
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    ControlObject *co = new ControlObject(ConfigKey("[Channel1]", "co"));
    co->set(0.0);
    scriptEngine->execute("setValue");
    ControlObject::sync();
    EXPECT_DOUBLE_EQ(co->get(), 1.0f);

    delete co;
    co = NULL;

    f.remove();
}

TEST_F(ControllerEngineTest, scriptGetSetValue) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("getSetValue = function() { var val = engine.getValue('[Channel1]', 'co'); engine.setValue('[Channel1]', 'co', val + 1); }\n");
    f.close();

    scriptEngine->evaluate(script);;
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    ControlObject *co = new ControlObject(ConfigKey("[Channel1]", "co"));
    co->set(0.0);
    scriptEngine->execute("getSetValue");
    ControlObject::sync();
    EXPECT_DOUBLE_EQ(co->get(), 1.0f);

    delete co;
    co = NULL;

    f.remove();
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlNamedFunction) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write(
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
        "};\n"
    );
    f.close();

    scriptEngine->evaluate(script);
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    EXPECT_TRUE(scriptEngine->execute("testConnectDisconnectControl"));
    EXPECT_TRUE(scriptEngine->execute("checkConnectDisconnectControl"));

    f.remove();
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlClosure) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write(
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
        "};\n"
    );
    f.close();

    scriptEngine->evaluate(script);
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    EXPECT_TRUE(scriptEngine->execute("testConnectDisconnectControl"));
    EXPECT_TRUE(scriptEngine->execute("checkConnectDisconnectControl"));

    f.remove();
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlIsDisconnected) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write(
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
        "};\n"
    );
    f.close();

    scriptEngine->evaluate(script);
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    EXPECT_TRUE(scriptEngine->execute("testConnectDisconnectControl"));
    EXPECT_TRUE(scriptEngine->execute("checkConnectDisconnectControl"));

    f.remove();
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlIsDisconnectedByName) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write(
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
        "};\n"
    );
    f.close();

    scriptEngine->evaluate(script);
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    EXPECT_TRUE(scriptEngine->execute("testConnectDisconnectControl"));
    EXPECT_TRUE(scriptEngine->execute("checkConnectDisconnectControl"));

    f.remove();
}

TEST_F(ControllerEngineTest, scriptConnectDisconnectControlIsDisconnectedByObject) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write(
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
        "};\n"
    );
    f.close();

    scriptEngine->evaluate(script);
    EXPECT_FALSE(scriptEngine->hasErrors(script));

    EXPECT_TRUE(scriptEngine->execute("testConnectDisconnectControl"));
    EXPECT_TRUE(scriptEngine->execute("checkConnectDisconnectControl"));

    f.remove();
}

}
