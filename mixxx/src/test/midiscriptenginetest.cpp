
#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QFile>

#include "controlobject.h"
#include "configobject.h"
#include "midi/midiscriptengine.h"

namespace {

class MidiScriptEngineTest : public testing::Test {
  protected:
    virtual void SetUp() {
        qDebug() << "SetUp";
        static int argc = 1;
        static char** argv = NULL;
        app = new QApplication(argc, argv);
        MidiDevice* pDevice = NULL;
        scriptEngine = new MidiScriptEngine(pDevice);
        scriptEngine->setMidiDebug(false);
        scriptEngine->setMidiPopups(false);
        scriptEngine->moveToThread(scriptEngine);
        scriptEngine->start();
        while(!scriptEngine->isReady()) { }
    }

    virtual void TearDown() {
        qDebug() << "TearDown";
        scriptEngine->gracefulShutdown(QList<QString>());
        scriptEngine->wait();
        delete scriptEngine;
        delete app;
    }

    QApplication *app;
    MidiScriptEngine *scriptEngine;
};

TEST_F(MidiScriptEngineTest, commonScriptHasNoErrors) {
    // ConfigObject<ConfigValue> config("~/.mixxx/mixxx.cfg");
    // QString commonScript = config.getConfigPath() + "/" +
    //         "/midi/midi-mappings-scripts.js";
    QString commonScript = "./res/midi/midi-mappings-scripts.js";
    scriptEngine->evaluate(commonScript);
    EXPECT_FALSE(scriptEngine->hasErrors(commonScript));
}

TEST_F(MidiScriptEngineTest, scriptSetValue) {
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

TEST_F(MidiScriptEngineTest, scriptGetSetValue) {
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

}
