
#include <gtest/gtest.h>
#include <QDebug>
#include <QApplication>
#include <QObject>
#include <QFile>

#include "controlobject.h"
#include "script/midiscriptengine.h"
#include "midiobjectnull.h"

#define UNIX_SHARE_PATH "/usr/share/mixxx"

namespace {
    class MidiScriptEngineTest : public testing::Test {
    protected:
        MidiScriptEngineTest() {
        }

        virtual void SetUp() {
            static int argc = 1;
            static char** argv = NULL;
            app = new QApplication(argc, argv);
            midiObject = new MidiObjectNull();
            scriptEngine = new MidiScriptEngine(midiObject);
            scriptEngine->moveToThread(scriptEngine);
            scriptEngine->start();
            while(!scriptEngine->isReady()) { }
        }

        virtual void TearDown() {
            delete scriptEngine;
            delete midiObject;
            delete app;
        }

        QApplication *app;
        MidiObject* midiObject;
        MidiScriptEngine *scriptEngine;
    };

    TEST_F(MidiScriptEngineTest, commonScriptHasNoErrors) {
        QString commonScript = QString(UNIX_SHARE_PATH) +
            "/midi/midi-mappings-scripts.js";
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
