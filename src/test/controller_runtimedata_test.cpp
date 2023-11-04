#include <QtDebug>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/controllerruntimedata.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "controllers/scripting/legacy/scriptconnection.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/time.h"

const RuntimeLoggingCategory logger(QString("test").toLocal8Bit());

class ControllerRuntimeDataTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        pRuntimeData = std::make_shared<ControllerRuntimeData>(nullptr);
        cEngineA = new ControllerScriptEngineLegacy(nullptr, logger);
        cEngineA->setRuntimeData(pRuntimeData);
        cEngineA->initialize();
        cEngineB = new ControllerScriptEngineLegacy(nullptr, logger);
        cEngineA->setRuntimeData(pRuntimeData);
        cEngineB->initialize();
    }

    void TearDown() override {
        delete cEngineA;
        delete cEngineB;
        mixxx::Time::setTestMode(false);
    }

    QJSValue evaluateA(const QString& code) {
        return cEngineA->jsEngine()->evaluate(code);
    }

    QJSValue evaluateB(const QString& code) {
        return cEngineA->jsEngine()->evaluate(code);
    }

    std::shared_ptr<QJSEngine> jsEngineA() {
        return cEngineA->jsEngine();
    }

    std::shared_ptr<QJSEngine> jsEngineB() {
        return cEngineB->jsEngine();
    }

    const QList<ScriptConnection>& runtimeDataConnectionsEngineA() {
        return static_cast<ControllerScriptInterfaceLegacy*>(
                jsEngineA()->globalObject().property("engine").toQObject())
                ->m_runtimeDataConnections;
    }

    const QList<ScriptConnection>& runtimeDataConnectionsEngineB() {
        return static_cast<ControllerScriptInterfaceLegacy*>(
                jsEngineB()->globalObject().property("engine").toQObject())
                ->m_runtimeDataConnections;
    }

    ControllerScriptEngineLegacy* cEngineA;
    ControllerScriptEngineLegacy* cEngineB;

    std::shared_ptr<ControllerRuntimeData> pRuntimeData;
};

TEST_F(ControllerRuntimeDataTest, getSetRuntimeData) {
    pRuntimeData->set(QVariant("foobar"));
    EXPECT_TRUE(!evaluateA(R"--(
let data = engine.getRuntimeData();
if (data !== "foobar") throw "Something is wrong";
engine.setRuntimeData("barfoo");
)--")
                         .isError());
    auto data = pRuntimeData->get();
    EXPECT_TRUE(data.canConvert<QString>());
    EXPECT_EQ(data.toString(), "barfoo");

    EXPECT_TRUE(!evaluateB(R"--(
let data = engine.getRuntimeData();
if (data !== "barfoo") throw "Something is wrong";
engine.setRuntimeData("bazfuu");
)--")
                         .isError());
    data = pRuntimeData->get();
    EXPECT_TRUE(data.canConvert<QString>());
    EXPECT_EQ(data.toString(), "bazfuu");
}

TEST_F(ControllerRuntimeDataTest, runtimeDataCallback) {
    EXPECT_TRUE(!evaluateA(R"--(
engine.onRuntimeDataUpdate((data) => {
    if (data !== "foobar") throw "Something is wrong";
    engine.setRuntimeData("bazfuu")
});
)--")
                         .isError());
    pRuntimeData->set(QVariant("foobar"));
    application()->processEvents();

    auto data = pRuntimeData->get();
    EXPECT_TRUE(data.canConvert<QString>());
    EXPECT_EQ(data.toString(), "bazfuu");
}

TEST_F(ControllerRuntimeDataTest, canTrigger) {
    EXPECT_TRUE(!evaluateA(R"--(
engine.onRuntimeDataUpdate((data) => {
    if (data) return;
    engine.setRuntimeData("bazfuu")
}).trigger();
)--")
                         .isError());
    application()->processEvents();

    auto data = pRuntimeData->get();
    EXPECT_TRUE(data.canConvert<QString>());
    EXPECT_EQ(data.toString(), "bazfuu");
}

TEST_F(ControllerRuntimeDataTest, canConnectDisconnect) {
    EXPECT_TRUE(!evaluateA(R"--(
let con = engine.onRuntimeDataUpdate((data) => {
    throw "Something is wrong";
});
if (!con.isConnected) throw "Something is wrong";
con.disconnect()
if (con.isConnected) throw "Something is wrong";
)--")
                         .isError());
    pRuntimeData->set(QVariant("foobar"));
    application()->processEvents();

    EXPECT_TRUE(runtimeDataConnectionsEngineA().isEmpty());
}
