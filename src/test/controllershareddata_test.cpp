#include "controllers/controllershareddata.h"

#include <gtest/gtest.h>

#include <QtDebug>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
#include "controllers/scripting/legacy/shareddataconnection.h"
#include "controllers/softtakeover.h"
#include "preferences/usersettings.h"
#include "test/mixxxtest.h"
#include "util/color/colorpalette.h"
#include "util/time.h"

using namespace std::chrono_literals;

const RuntimeLoggingCategory kTestLogger(QString("test").toLocal8Bit());

//
// ── Unit tests for the ControllerSharedData backend ─────────────────────────
//

class ControllerSharedDataBackendTest : public ::testing::Test {
  protected:
    void SetUp() override {
        m_pSharedData = std::make_unique<ControllerSharedData>();
    }

    std::unique_ptr<ControllerSharedData> m_pSharedData;
};

TEST_F(ControllerSharedDataBackendTest, GetReturnsNullForMissingNamespace) {
    QVariant val = m_pSharedData->get("ns1", "deck1", "shift");
    EXPECT_FALSE(val.isValid());
}

TEST_F(ControllerSharedDataBackendTest, GetReturnsNullForMissingEntity) {
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true));
    QVariant val = m_pSharedData->get("ns1", "deck2", "shift");
    EXPECT_FALSE(val.isValid());
}

TEST_F(ControllerSharedDataBackendTest, GetReturnsNullForMissingKey) {
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true));
    QVariant val = m_pSharedData->get("ns1", "deck1", "filter");
    EXPECT_FALSE(val.isValid());
}

TEST_F(ControllerSharedDataBackendTest, SetAndGetBool) {
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true));
    QVariant val = m_pSharedData->get("ns1", "deck1", "shift");
    EXPECT_TRUE(val.isValid());
    EXPECT_TRUE(val.toBool());
}

TEST_F(ControllerSharedDataBackendTest, SetAndGetNumber) {
    m_pSharedData->set("ns1", "mixer", "crossfader", QVariant(0.75));
    QVariant val = m_pSharedData->get("ns1", "mixer", "crossfader");
    EXPECT_TRUE(val.isValid());
    EXPECT_DOUBLE_EQ(val.toDouble(), 0.75);
}

TEST_F(ControllerSharedDataBackendTest, SetAndGetString) {
    m_pSharedData->set("ns1", "controller", "mode", QVariant("performance"));
    QVariant val = m_pSharedData->get("ns1", "controller", "mode");
    EXPECT_TRUE(val.isValid());
    EXPECT_EQ(val.toString(), QString("performance"));
}

TEST_F(ControllerSharedDataBackendTest, OverwriteValue) {
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true));
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(false));
    QVariant val = m_pSharedData->get("ns1", "deck1", "shift");
    EXPECT_FALSE(val.toBool());
}

TEST_F(ControllerSharedDataBackendTest, NamespacesAreIsolated) {
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true));
    m_pSharedData->set("ns2", "deck1", "shift", QVariant(false));
    QVariant val1 = m_pSharedData->get("ns1", "deck1", "shift");
    QVariant val2 = m_pSharedData->get("ns2", "deck1", "shift");
    EXPECT_TRUE(val1.toBool());
    EXPECT_FALSE(val2.toBool());
}

TEST_F(ControllerSharedDataBackendTest, MultipleEntitiesAndKeys) {
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true));
    m_pSharedData->set("ns1", "deck1", "filter", QVariant(0.5));
    m_pSharedData->set("ns1", "deck2", "shift", QVariant(false));
    m_pSharedData->set("ns1", "mixer", "crossfader", QVariant(0.0));

    EXPECT_TRUE(m_pSharedData->get("ns1", "deck1", "shift").toBool());
    EXPECT_DOUBLE_EQ(m_pSharedData->get("ns1", "deck1", "filter").toDouble(), 0.5);
    EXPECT_FALSE(m_pSharedData->get("ns1", "deck2", "shift").toBool());
    EXPECT_DOUBLE_EQ(m_pSharedData->get("ns1", "mixer", "crossfader").toDouble(), 0.0);
}

TEST_F(ControllerSharedDataBackendTest, UpdateSignalEmitted) {
    QString receivedNs;
    QString receivedEntity;
    QString receivedKey;
    QVariant receivedValue;
    QObject* receivedSender = nullptr;

    QObject::connect(m_pSharedData.get(),
            &ControllerSharedData::updated,
            [&](const QString& ns,
                    const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                receivedNs = ns;
                receivedEntity = entity;
                receivedKey = key;
                receivedValue = value;
                receivedSender = sender;
            });

    QObject senderObj;
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true), &senderObj);

    EXPECT_EQ(receivedNs, "ns1");
    EXPECT_EQ(receivedEntity, "deck1");
    EXPECT_EQ(receivedKey, "shift");
    EXPECT_TRUE(receivedValue.toBool());
    EXPECT_EQ(receivedSender, &senderObj);
}

//
// ── Unit tests for ControllerNamespacedSharedData ───────────────────────────
//

class NamespacedSharedDataTest : public ::testing::Test {
  protected:
    void SetUp() override {
        m_pSharedData = std::make_unique<ControllerSharedData>();
        m_pNamespaced.reset(m_pSharedData->namespaced("testNS"));
    }

    std::unique_ptr<ControllerSharedData> m_pSharedData;
    std::unique_ptr<ControllerNamespacedSharedData> m_pNamespaced;
};

TEST_F(NamespacedSharedDataTest, GetReturnsNullForMissing) {
    QVariant val = m_pNamespaced->get("deck1", "shift");
    EXPECT_FALSE(val.isValid());
}

TEST_F(NamespacedSharedDataTest, SetAndGet) {
    m_pNamespaced->set("deck1", "shift", QVariant(true));
    QVariant val = m_pNamespaced->get("deck1", "shift");
    EXPECT_TRUE(val.toBool());
}

TEST_F(NamespacedSharedDataTest, ReadFromOtherStore) {
    // Set via the parent store, read via namespace wrapper
    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    QVariant val = m_pNamespaced->get("deck1", "shift");
    EXPECT_TRUE(val.toBool());
    // Set via wrapper, read via parent store
    m_pNamespaced->set("deck2", "shift", QVariant(true));
    val = m_pSharedData->get("testNS", "deck2", "shift");
    EXPECT_TRUE(val.toBool());
}

TEST_F(NamespacedSharedDataTest, OnlyReceivesOwnNamespace) {
    QString receivedEntity;
    int callCount = 0;

    QObject::connect(m_pNamespaced.get(),
            &ControllerNamespacedSharedData::updated,
            [&](const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                Q_UNUSED(key);
                Q_UNUSED(value);
                Q_UNUSED(sender);
                receivedEntity = entity;
                callCount++;
            });

    m_pSharedData->set("otherNS", "deck1", "shift", QVariant(true));
    EXPECT_EQ(callCount, 0);

    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(receivedEntity, "deck1");
}

TEST_F(NamespacedSharedDataTest, UpdateSignalIncludesSender) {
    QObject* receivedSender = nullptr;

    QObject::connect(m_pNamespaced.get(),
            &ControllerNamespacedSharedData::updated,
            [&](const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                Q_UNUSED(entity);
                Q_UNUSED(key);
                Q_UNUSED(value);
                receivedSender = sender;
            });

    QObject senderObj;
    m_pNamespaced->set("deck1", "shift", QVariant(true), &senderObj);
    EXPECT_EQ(receivedSender, &senderObj);
}

//
// ── Integration tests with the JS engine ─────────────────────────────────────
//

class ControllerSharedDataTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::addTestTime(10ms);
        m_pSharedData = std::make_shared<ControllerSharedData>(nullptr);
        m_pEngineA = new ControllerScriptEngineLegacy(nullptr, kTestLogger);
        m_pEngineA->setSharedData(m_pSharedData->namespaced("testNS"));
        m_pEngineA->initialize();
        m_pEngineB = new ControllerScriptEngineLegacy(nullptr, kTestLogger);
        m_pEngineB->setSharedData(m_pSharedData->namespaced("testNS"));
        m_pEngineB->initialize();
    }

    void TearDown() override {
        delete m_pEngineA;
        delete m_pEngineB;
        mixxx::Time::setTestMode(false);
    }

    QJSValue evaluateA(const QString& code) {
        return m_pEngineA->jsEngine()->evaluate(code);
    }

    QJSValue evaluateB(const QString& code) {
        return m_pEngineB->jsEngine()->evaluate(code);
    }

    std::shared_ptr<QJSEngine> jsEngineA() {
        return m_pEngineA->jsEngine();
    }

    std::shared_ptr<QJSEngine> jsEngineB() {
        return m_pEngineB->jsEngine();
    }

    ControllerScriptInterfaceLegacy* interfaceA() {
        return static_cast<ControllerScriptInterfaceLegacy*>(
                jsEngineA()->globalObject().property("engine").toQObject());
    }

    ControllerScriptInterfaceLegacy* interfaceB() {
        return static_cast<ControllerScriptInterfaceLegacy*>(
                jsEngineB()->globalObject().property("engine").toQObject());
    }

    const QList<SharedDataConnection>& sharedDataConnectionsA() {
        return interfaceA()->m_sharedDataConnections;
    }

    const QList<SharedDataConnection>& sharedDataConnectionsB() {
        return interfaceB()->m_sharedDataConnections;
    }

    ControllerScriptEngineLegacy* m_pEngineA;
    ControllerScriptEngineLegacy* m_pEngineB;
    std::shared_ptr<ControllerSharedData> m_pSharedData;
};

TEST_F(ControllerSharedDataTest, GetSetSharedValue) {
    // Set from C++, read from JS
    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    auto result = evaluateA(R"--(
        let val = engine.getSharedValue("deck1", "shift");
        if (val !== true) throw new Error("Expected true, got " + val);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    // Set from JS, read from C++
    result = evaluateA(R"--(
        engine.setSharedValue("deck1", "shift", false);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
    EXPECT_FALSE(m_pSharedData->get("testNS", "deck1", "shift").toBool());
}

TEST_F(ControllerSharedDataTest, GetSetNumber) {
    auto result = evaluateA(R"--(
        engine.setSharedValue("mixer", "crossfader", 0.75);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
    EXPECT_DOUBLE_EQ(m_pSharedData->get("testNS", "mixer", "crossfader").toDouble(), 0.75);

    result = evaluateA(R"--(
        let val = engine.getSharedValue("mixer", "crossfader");
        if (val !== 0.75) throw new Error("Expected 0.75, got " + val);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, GetSetString) {
    auto result = evaluateA(R"--(
        engine.setSharedValue("controller", "mode", "performance");
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
    EXPECT_EQ(m_pSharedData->get("testNS", "controller", "mode").toString(),
            QString("performance"));

    result = evaluateA(R"--(
        let val = engine.getSharedValue("controller", "mode");
        if (val !== "performance") throw new Error("Expected performance, got " + val);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, GetNonExistentReturnsUndefined) {
    auto result = evaluateA(R"--(
        let val = engine.getSharedValue("deck1", "nonexistent");
        if (val !== undefined) throw new Error("Expected undefined, got " + typeof val);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, CrossEngineSharedValue) {
    // Set from engine A, read from engine B
    auto result = evaluateA(R"--(
        engine.setSharedValue("deck1", "shift", true);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    result = evaluateB(R"--(
        let val = engine.getSharedValue("deck1", "shift");
        if (val !== true) throw new Error("Expected true, got " + val);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, SharedValueConnectionCallback) {
    // Engine B subscribes, engine A changes value.
    // Engine B's callback should fire.
    auto result = evaluateB(R"--(
        var receivedValue = undefined;
        var receivedEntity = undefined;
        var receivedKey = undefined;
        engine.makeSharedValueConnection("deck1", "shift", function(value, entity, key) {
            receivedValue = value;
            receivedEntity = entity;
            receivedKey = key;
        });
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    // Engine A sets the value
    result = evaluateA(R"--(
        engine.setSharedValue("deck1", "shift", true);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
    application()->processEvents();

    // Verify the callback was invoked with correct args
    result = evaluateB(R"--(
        if (receivedValue !== true) throw new Error("Expected true, got " + receivedValue);
        if (receivedEntity !== "deck1") throw new Error("Expected deck1, got " + receivedEntity);
        if (receivedKey !== "shift") throw new Error("Expected shift, got " + receivedKey);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, SelfNotificationSuppressed) {
    // Engine A subscribes and then sets the value itself.
    // The callback should NOT fire because self-notifications are suppressed.
    auto result = evaluateA(R"--(
        var callbackCalled = false;
        engine.makeSharedValueConnection("deck1", "shift", function(value, entity, key) {
            callbackCalled = true;
        });
        engine.setSharedValue("deck1", "shift", true);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
    application()->processEvents();

    result = evaluateA(R"--(
        if (callbackCalled) throw new Error("Callback should not have been called for self-update");
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, ConnectionOnlyFiresForMatchingEntityKey) {
    auto result = evaluateA(R"--(
        var callCount = 0;
        engine.makeSharedValueConnection("deck1", "shift", function(value) {
            callCount++;
        });
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    // Set different entity/key from C++ (same namespace) — shouldn't trigger
    m_pSharedData->set("testNS", "deck2", "shift", QVariant(true));
    application()->processEvents();

    result = evaluateA(R"--(
        if (callCount !== 0) throw new Error("Expected 0 calls, got " + callCount);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    // Set matching entity/key from C++ — should trigger
    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    application()->processEvents();

    result = evaluateA(R"--(
        if (callCount !== 1) throw new Error("Expected 1 call, got " + callCount);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, ConnectionCanDisconnect) {
    auto result = evaluateA(R"--(
        var callbackCalled = false;
        var conn = engine.makeSharedValueConnection("deck1", "shift", function(value) {
            callbackCalled = true;
        });
        if (!conn.isConnected) throw new Error("Connection should start connected");
        conn.disconnect();
        if (conn.isConnected) throw new Error("Connection should be disconnected");
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    // Set value from C++ — callback should NOT fire after disconnect
    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    application()->processEvents();

    result = evaluateA(R"--(
        if (callbackCalled) throw new Error("Callback should not fire after disconnect");
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    EXPECT_TRUE(sharedDataConnectionsA().isEmpty());
}

TEST_F(ControllerSharedDataTest, ConnectionCanTrigger) {
    auto result = evaluateA(R"--(
        var triggerCalled = false;
        var conn = engine.makeSharedValueConnection("deck1", "shift", function(value) {
            triggerCalled = true;
        });
        conn.trigger();
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    result = evaluateA(R"--(
        if (!triggerCalled) throw new Error("trigger() should have called callback");
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}

TEST_F(ControllerSharedDataTest, NamespacePreventsCrossNamespaceLeak) {
    // Create a separate engine with a different namespace
    auto* pEngineC = new ControllerScriptEngineLegacy(nullptr, kTestLogger);
    pEngineC->setSharedData(m_pSharedData->namespaced("otherNS"));
    pEngineC->initialize();

    // Engine A sets deck1/shift in testNS
    auto result = evaluateA(R"--(
        engine.setSharedValue("deck1", "shift", true);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    // Verify the value exists in testNS
    EXPECT_TRUE(m_pSharedData->get("testNS", "deck1", "shift").toBool());

    // Engine C (otherNS) should not see it
    auto resultC = pEngineC->jsEngine()->evaluate(R"--(
        let val = engine.getSharedValue("deck1", "shift");
        if (val !== undefined) throw new Error("Expected undefined in other namespace, got " + val);
    )--");
    EXPECT_FALSE(resultC.isError()) << resultC.toString().toStdString();

    delete pEngineC;
}

TEST_F(ControllerSharedDataTest, SetAndGetArrayValue) {
    auto result = evaluateA(R"--(
        engine.setSharedValue("deck1", "hotcues", [1, 2, 3]);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    QVariant val = m_pSharedData->get("testNS", "deck1", "hotcues");
    EXPECT_TRUE(val.isValid());
    QVariantList list = val.toList();
    EXPECT_EQ(list.size(), 3);
}

TEST_F(ControllerSharedDataTest, SetAndGetNullValue) {
    auto result = evaluateA(R"--(
        engine.setSharedValue("deck1", "status", null);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    QVariant val = m_pSharedData->get("testNS", "deck1", "status");
    EXPECT_TRUE(val.isValid());
    EXPECT_TRUE(val.isNull());
}

TEST_F(ControllerSharedDataTest, MultipleConnectionsSameEntityKey) {
    auto result = evaluateB(R"--(
        var count1 = 0;
        var count2 = 0;
        engine.makeSharedValueConnection("deck1", "shift", function(value) {
            count1++;
        });
        engine.makeSharedValueConnection("deck1", "shift", function(value) {
            count2++;
        });
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();

    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    application()->processEvents();

    result = evaluateB(R"--(
        if (count1 !== 1) throw new Error("Expected count1=1, got " + count1);
        if (count2 !== 1) throw new Error("Expected count2=1, got " + count2);
    )--");
    EXPECT_FALSE(result.isError()) << result.toString().toStdString();
}
