
#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QFile>

#include "controlobject.h"
#include "configobject.h"
#include "controllers/controllerengine.h"

namespace {

class ControllerEngineTest : public testing::Test {
  protected:
    virtual void SetUp() {
        qDebug() << "SetUp";
        static int argc = 1;
        static char** argv = NULL;
        app = new QApplication(argc, argv);
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
    // ConfigObject<ConfigValue> config("~/.mixxx/mixxx.cfg");
    // QString commonScript = config.getConfigPath() + "/" +
    //         "/midi/midi-mappings-scripts.js";
    QString commonScript = "./res/controllers/common-controller-scripts.js";
    cEngine->evaluate(commonScript);
    EXPECT_FALSE(cEngine->hasErrors(commonScript));
}

TEST_F(ControllerEngineTest, scriptSetValue) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("setValue = function() { engine.setValue('[Channel1]', 'co', 1.0); }\n");
    f.close();

    cEngine->evaluate(script);
    EXPECT_FALSE(cEngine->hasErrors(script));

    ControlObject *co = new ControlObject(ConfigKey("[Channel1]", "co"));
    co->set(0.0);
    cEngine->execute("setValue");
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

    cEngine->evaluate(script);
    EXPECT_FALSE(cEngine->hasErrors(script));

    ControlObject *co = new ControlObject(ConfigKey("[Channel1]", "co"));
    co->set(0.0);
    cEngine->execute("getSetValue");
    ControlObject::sync();
    EXPECT_DOUBLE_EQ(co->get(), 1.0f);

    delete co;
    co = NULL;

    f.remove();
}

TEST_F(ControllerEngineTest, setInvalidControlObject) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("setValue = function() { engine.setValue('[Nothing]', 'nothing', 1.0); }\n");
    f.close();

    cEngine->evaluate(script);
    EXPECT_FALSE(cEngine->hasErrors(script));

    EXPECT_TRUE(cEngine->execute("setValue"));

    f.remove();
}

TEST_F(ControllerEngineTest, getInvalidControlObject) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("getValue = function() { return engine.getValue('[Nothing]', 'nothing'); }\n");
    f.close();
    
    cEngine->evaluate(script);
    EXPECT_FALSE(cEngine->hasErrors(script));

    EXPECT_TRUE(cEngine->execute("getValue"));
    
    f.remove();
}

TEST_F(ControllerEngineTest, automaticReaction) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("setUp = function() { engine.connectControl('[Channel1]','co','reaction'); }\n");
    f.write("reaction = function(value) { if (value == 2.5) print('TEST PASSED: '+value);\
                                          else print('TEST FAILED!  TEST FAILED!  TEST FAILED: '+value);\
                                          return value; }\n");
    f.close();

    cEngine->evaluate(script);
    EXPECT_FALSE(cEngine->hasErrors(script));

    ControlObject *co = new ControlObject(ConfigKey("[Channel1]", "co"));
    co->set(0.0);
    EXPECT_TRUE(cEngine->execute("setUp"));
    ControlObject::sync();

    // The actual test
    //  TODO: Have the JS call a function in this test class so the test framework
    //  can tell if it actually passed or not
    co->set(2.5);
    ControlObject::sync();

    f.remove();
}

// Can't test timed reactions without an event loop!
/*
TEST_F(ControllerEngineTest, oneShotTimedReaction) {
    QString script = "test.js";
    QFile f(script);
    f.open(QIODevice::ReadWrite | QIODevice::Truncate);
    f.write("function global() {}\n");
    f.write("setUp = function() { global.date = new Date(); engine.beginTimer(250,'reaction()',true); }\n");
    f.write("reaction = function() { if ((new Date()-global.date) == 250) print('TEST PASSED');\
                                     else print('TEST FAILED!  TEST FAILED!  TEST FAILED: '+(new Date()-global.date));\
                                     return (new Date()-global.date); }\n");
    f.close();
    
    cEngine->evaluate(script);
    EXPECT_FALSE(cEngine->hasErrors(script));

    EXPECT_TRUE(cEngine->execute("setUp"));
    usleep(500*1000);

    // Test passes if the JS prints it after 250ms
    //  TODO: Have the JS call a function in this test class

    f.remove();
}
*/

}
