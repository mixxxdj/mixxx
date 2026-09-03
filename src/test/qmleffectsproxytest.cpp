#include <gtest/gtest.h>

#include <QSignalSpy>
#include <memory>

#include "effects/backends/builtin/echoeffect.h"
#include "effects/backends/builtin/parametriceqeffect.h"
#include "effects/effectchain.h"
#include "effects/effectsmanager.h"
#include "effects/presets/effectchainpreset.h"
#include "effects/presets/effectchainpresetmanager.h"
#include "engine/channelhandle.h"
#include "qml/qmlchainpresetmodel.h"
#include "qml/qmleffectslotparametersmodel.h"
#include "qml/qmleffectslotproxy.h"
#include "qml/qmleffectsmanagerproxy.h"
#include "qml/qmleffectunitproxy.h"
#include "test/mixxxtest.h"

namespace {

class QmlEffectsProxyTest : public MixxxTest {
  protected:
    void SetUp() override {
        auto pChannelHandleFactory = std::make_shared<ChannelHandleFactory>();
        m_pEffectsManager =
                std::make_shared<EffectsManager>(config(), pChannelHandleFactory);
        const QString mainOutputGroup = QStringLiteral("[MasterOutput]");
        m_pEffectsManager->registerInputChannel(ChannelHandleAndGroup(
                pChannelHandleFactory->getOrCreateHandle(mainOutputGroup), mainOutputGroup));
        m_pEffectsManager->setup();
        m_pProxy = std::make_unique<mixxx::qml::QmlEffectsManagerProxy>(
                m_pEffectsManager);
    }

    void TearDown() override {
        m_pProxy.reset();
        for (int unitIndex = 0; unitIndex < kNumStandardEffectUnits; ++unitIndex) {
            const auto pChain = m_pEffectsManager->getStandardEffectChain(unitIndex);
            auto pEmptyPreset = EffectChainPresetPointer::create();
            pEmptyPreset->setName(QString());
            pChain->loadChainPreset(pEmptyPreset);
            EXPECT_TRUE(pChain->isEmpty());
            EXPECT_TRUE(pChain->presetName().isEmpty());
        }
        m_pEffectsManager.reset();
    }

    std::shared_ptr<EffectsManager> m_pEffectsManager;
    std::unique_ptr<mixxx::qml::QmlEffectsManagerProxy> m_pProxy;
};

TEST_F(QmlEffectsProxyTest, UnitLookup) {
    auto* pUnit1 = m_pProxy->getEffectUnit(1);
    ASSERT_NE(nullptr, pUnit1);
    EXPECT_EQ(pUnit1, m_pProxy->getEffectUnit(1));
    EXPECT_EQ(1, pUnit1->getUnitNumber());
    EXPECT_EQ(QStringLiteral("[EffectRack1_EffectUnit1]"), pUnit1->getGroup());
    EXPECT_EQ(nullptr, m_pProxy->getEffectUnit(0));
    EXPECT_EQ(nullptr, m_pProxy->getEffectUnit(5));
}

TEST_F(QmlEffectsProxyTest, PresetModelsResetSeparately) {
    auto* pStandardModel = m_pProxy->property("standardChainPresetModel")
                                   .value<mixxx::qml::QmlChainPresetModel*>();
    auto* pQuickModel = m_pProxy->property("quickChainPresetModel")
                                .value<mixxx::qml::QmlChainPresetModel*>();
    ASSERT_NE(nullptr, pStandardModel);
    ASSERT_NE(nullptr, pQuickModel);
    EXPECT_GT(pStandardModel->rowCount({}), 0);
    EXPECT_GT(pQuickModel->rowCount({}), 0);
    EXPECT_TRUE(pStandardModel->roleNames().values().contains("name"));
    EXPECT_TRUE(pStandardModel->roleNames().values().contains("readOnly"));

    QSignalSpy standardResetSpy(pStandardModel, &QAbstractItemModel::modelReset);
    QSignalSpy quickResetSpy(pQuickModel, &QAbstractItemModel::modelReset);
    QMetaObject::invokeMethod(m_pEffectsManager->getChainPresetManager().get(),
            "effectChainPresetListUpdated");
    EXPECT_EQ(1, standardResetSpy.count());
    EXPECT_EQ(0, quickResetSpy.count());
    QMetaObject::invokeMethod(m_pEffectsManager->getChainPresetManager().get(),
            "quickEffectChainPresetListUpdated");
    EXPECT_EQ(1, standardResetSpy.count());
    EXPECT_EQ(1, quickResetSpy.count());
}

TEST_F(QmlEffectsProxyTest, UnitPresetStateAndLoading) {
    auto* pUnit1 = m_pProxy->getEffectUnit(1);
    ASSERT_NE(nullptr, pUnit1);
    QSignalSpy presetSpy(pUnit1, &mixxx::qml::QmlEffectUnitProxy::presetChanged);
    pUnit1->loadPreset(-1);
    EXPECT_EQ(0, presetSpy.count());
    pUnit1->loadPreset(0);
    EXPECT_EQ(0, pUnit1->getPresetIndex());
    EXPECT_FALSE(pUnit1->getPresetName().isEmpty());
    EXPECT_EQ(!pUnit1->isPresetReadOnly(), pUnit1->canUpdatePreset());
    EXPECT_EQ(pUnit1->canUpdatePreset() && !pUnit1->getPresetName().isEmpty(),
            pUnit1->canRenamePreset());
    EXPECT_GE(presetSpy.count(), 1);
}

TEST_F(QmlEffectsProxyTest, SlotEffectAndParameterVisibility) {
    constexpr int kExpectedParameterCount = 6;
    constexpr int kParameter1Index = 0;
    constexpr int kParameter2Index = 1;
    constexpr int kParameter3Index = 2;
    constexpr int kButtonParameter1Index = 4;

    std::unique_ptr<mixxx::qml::QmlEffectSlotProxy> pSlot(
            m_pProxy->getEffectSlot(1, 1));
    ASSERT_NE(nullptr, pSlot);
    QSignalSpy effectSpy(
            pSlot.get(), &mixxx::qml::QmlEffectSlotProxy::effectIdChanged);
    auto* pModel = pSlot->getParametersModel();
    ASSERT_NE(nullptr, pModel);
    QSignalSpy modelResetSpy(pModel, &QAbstractItemModel::modelReset);
    QSignalSpy dataChangedSpy(pModel, &QAbstractItemModel::dataChanged);

    pSlot->setEffectId(EchoEffect::getId());
    ASSERT_TRUE(pSlot->isLoaded());
    EXPECT_GE(effectSpy.count(), 1);
    EXPECT_GE(modelResetSpy.count(), 1);

    ASSERT_EQ(kExpectedParameterCount, pModel->rowCount({}));
    EXPECT_TRUE(pModel->roleNames().values().contains("unitString"));
    EXPECT_EQ(QStringLiteral("parameter1"),
            pModel->get(kParameter1Index).toMap().value("controlKey").toString());
    EXPECT_EQ(QStringLiteral("parameter2"),
            pModel->get(kParameter2Index).toMap().value("controlKey").toString());
    EXPECT_EQ(QStringLiteral("button_parameter1"),
            pModel->get(kButtonParameter1Index)
                    .toMap()
                    .value("controlKey")
                    .toString());

    pSlot->setParameterVisible(QStringLiteral("feedback_amount"), false);
    EXPECT_GE(dataChangedSpy.count(), 1);
    EXPECT_FALSE(pModel->get(kParameter2Index).toMap().value("loaded").toBool());
    EXPECT_TRUE(pModel->get(kParameter2Index)
                    .toMap()
                    .value("controlKey")
                    .toString()
                    .isEmpty());
    EXPECT_EQ(QStringLiteral("parameter2"),
            pModel->get(kParameter3Index).toMap().value("controlKey").toString());

    pSlot->setParameterVisible(QStringLiteral("feedback_amount"), true);
    EXPECT_TRUE(pModel->get(kParameter2Index).toMap().value("loaded").toBool());
    pSlot->saveDefaultSnapshot();

    pSlot->setEffectId(ParametricEQEffect::getId());
    EXPECT_EQ(QStringLiteral("dB"),
            pModel->get(kParameter1Index).toMap().value("unitString").toString());
}

} // namespace
