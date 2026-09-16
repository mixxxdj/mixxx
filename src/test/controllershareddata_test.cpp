#include "controllers/controllershareddata.h"

#include <gtest/gtest.h>

#include <QtDebug>
#include <memory>

#include "control/controlobject.h"
#include "control/controlpotmeter.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "controllers/scripting/legacy/controllerscriptinterfacelegacy.h"
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
    struct Result {
        QString ns;
        QString entity;
        QString key;
        QVariant value;
        QObject* sender = nullptr;
    };
    auto received = std::make_shared<Result>();

    QObject::connect(m_pSharedData.get(),
            &ControllerSharedData::updated,
            [&](const QString& ns,
                    const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                received->ns = ns;
                received->entity = entity;
                received->key = key;
                received->value = value;
                received->sender = sender;
            });

    QObject senderObj;
    m_pSharedData->set("ns1", "deck1", "shift", QVariant(true), &senderObj);

    EXPECT_EQ(received->ns, "ns1");
    EXPECT_EQ(received->entity, "deck1");
    EXPECT_EQ(received->key, "shift");
    EXPECT_TRUE(received->value.toBool());
    EXPECT_EQ(received->sender, &senderObj);
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
    struct Result {
        QString entity;
        int callCount = 0;
    };
    auto received = std::make_shared<Result>();

    QObject::connect(m_pNamespaced.get(),
            &ControllerNamespacedSharedData::updated,
            [&](const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                Q_UNUSED(key);
                Q_UNUSED(value);
                Q_UNUSED(sender);
                received->entity = entity;
                received->callCount++;
            });

    m_pSharedData->set("otherNS", "deck1", "shift", QVariant(true));
    EXPECT_EQ(received->callCount, 0);

    m_pSharedData->set("testNS", "deck1", "shift", QVariant(true));
    EXPECT_EQ(received->callCount, 1);
    EXPECT_EQ(received->entity, "deck1");
}

TEST_F(NamespacedSharedDataTest, UpdateSignalIncludesSender) {
    struct Result {
        QObject* sender = nullptr;
    };
    auto received = std::make_shared<Result>();

    QObject::connect(m_pNamespaced.get(),
            &ControllerNamespacedSharedData::updated,
            [&](const QString& entity,
                    const QString& key,
                    const QVariant& value,
                    QObject* sender) {
                Q_UNUSED(entity);
                Q_UNUSED(key);
                Q_UNUSED(value);
                received->sender = sender;
            });

    QObject senderObj;
    m_pNamespaced->set("deck1", "shift", QVariant(true), &senderObj);
    EXPECT_EQ(received->sender, &senderObj);
}
