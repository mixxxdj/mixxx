#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "controllers/softtakeover.h"
#include "effects/effectparameterslot.h"
#include "effects/effectchainslot.h"
#include "effects/effectrack.h"
#include "effects/effect.h"
#include "effects/effectslot.h"
#include "mixxxtest.h"
#include "test/baseeffecttest.h"
#include "util/time.h"
#include "control/controlproxy.h"

class MetaLinkTest : public BaseEffectTest {
  protected:
    MetaLinkTest()
            : m_master(m_factory.getOrCreateHandle("[Master]"), "[Master]"),
              m_headphone(m_factory.getOrCreateHandle("[Headphone]"), "[Headphone]") {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromNanos(0));
        m_pEffectsManager->registerInputChannel(m_master);
        m_pEffectsManager->registerInputChannel(m_headphone);
        registerTestBackend();

        EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                                  "org.mixxx.test.chain1"));
        int iRackNumber = 0;
        int iChainNumber = 0;
        int iEffectNumber = 0;

        StandardEffectRackPointer pRack = m_pEffectsManager->addStandardEffectRack();
        m_pChainSlot = pRack->getEffectChainSlot(iChainNumber);
        m_pChainSlot->loadEffectChainToSlot(pChain);
        m_pEffectSlot = m_pChainSlot->getEffectSlot(iEffectNumber);

        QString group = StandardEffectRack::formatEffectSlotGroupString(
            iRackNumber, iChainNumber, iEffectNumber);

        EffectManifestPointer pManifest(new EffectManifest());
        pManifest->setId("org.mixxx.test.effect");
        pManifest->setName("Test Effect");
        pManifest->setMetaknobDefault(0.0);

        EffectManifestParameterPointer low = pManifest->addParameter();
        low->setId("low");
        low->setName(QObject::tr("Low"));
        low->setDescription(QObject::tr("Gain for Low Filter"));
        low->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
        low->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
        low->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
        low->setNeutralPointOnScale(0.25);
        low->setDefault(1.0);
        low->setMinimum(0);
        low->setMaximum(1.0);

        registerTestEffect(pManifest, false);

        // Check the controls reflect the state of their loaded effect.
        EffectPointer pEffect = m_pEffectsManager->instantiateEffect(pManifest->id());

        m_pEffectSlot->loadEffect(pEffect, false);

        QString itemPrefix = EffectParameterSlot::formatItemPrefix(0);

        m_pControlValue.reset(new ControlProxy(group, itemPrefix));

        m_pControlLinkType.reset(new ControlProxy(group,
                itemPrefix + QString("_link_type")));

        m_pControlLinkInverse.reset(new ControlProxy(group,
                itemPrefix + QString("_link_inverse")));
    }

    ChannelHandleFactory m_factory;
    ChannelHandleAndGroup m_master;
    ChannelHandleAndGroup m_headphone;

    EffectSlotPointer m_pEffectSlot;
    EffectChainSlotPointer m_pChainSlot;
    QScopedPointer<ControlProxy> m_pControlValue;
    QScopedPointer<ControlProxy> m_pControlLinkType;
    QScopedPointer<ControlProxy> m_pControlLinkInverse;
};

TEST_F(MetaLinkTest, LinkDefault) {
    // default is not Linked, value must be unchanged
    m_pEffectSlot->syncSofttakeover();
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->slotEffectMetaParameter(1.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->slotEffectMetaParameter(0.5);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->slotEffectMetaParameter(0.3);
    EXPECT_EQ(1.0, m_pControlValue->get());
}

TEST_F(MetaLinkTest, LinkLinked) {
    m_pEffectSlot->syncSofttakeover();
    m_pControlLinkType->set(
        static_cast<double>(EffectManifestParameter::LinkType::LINKED));
    m_pEffectSlot->slotEffectMetaParameter(1.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->slotEffectMetaParameter(0.5);
    EXPECT_EQ(0.25, m_pControlValue->get());
}

TEST_F(MetaLinkTest, LinkLinkedInverse) {
    m_pEffectSlot->syncSofttakeover();
    m_pControlLinkType->set(
        static_cast<double>(EffectManifestParameter::LinkType::LINKED));
    m_pControlLinkInverse->set(1.0);
    m_pEffectSlot->slotEffectMetaParameter(0.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->slotEffectMetaParameter(0.5);
    EXPECT_EQ(0.25, m_pControlValue->get());
}

TEST_F(MetaLinkTest, MetaToParameter_Softtakeover_EffectEnabled) {
    m_pControlLinkType->set(
        static_cast<double>(EffectManifestParameter::LinkType::LINKED));
    // Soft takeover should only occur when the effect is enabled.
    m_pEffectSlot->slotEnabled(1.0);

    // Soft takeover always ignores the first change.
    m_pEffectSlot->slotEffectMetaParameter(0.5);
    EXPECT_EQ(1.0, m_pControlValue->get());

    m_pControlValue->set(0.0);

    // Let enough time pass by to exceed soft-takeover's override interval.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() +
                             mixxx::Duration::fromMillis(2));

    // Ignored by SoftTakeover since it is too far from the current
    // parameter value of 0.0.
    m_pEffectSlot->slotEffectMetaParameter(1.0);
    EXPECT_EQ(0.0, m_pControlValue->get());
}

TEST_F(MetaLinkTest, MetaToParameter_Softtakeover_EffectDisabled) {
    // Soft takeover should not occur when the effect is disabled;
    // parameter values should always jump to match the metaknob.
    // Effects are disabled by default.
    m_pControlLinkType->set(
        static_cast<double>(EffectManifestParameter::LinkType::LINKED));

    m_pEffectSlot->slotEffectMetaParameter(1.0);
    EXPECT_EQ(1.0, m_pControlValue->get());

    m_pControlValue->set(0.0);

    // Let enough time pass by to exceed soft-takeover's override interval.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() +
                             mixxx::Duration::fromMillis(2));

    m_pEffectSlot->slotEffectMetaParameter(1.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
}

TEST_F(MetaLinkTest, SuperToMeta_Softtakeover_EffectEnabled) {
    // Soft takeover should only occur when the effect is enabled.
    m_pEffectSlot->slotEnabled(1.0);

    // Soft takeover always ignores the first change.
    m_pChainSlot->setSuperParameter(0.5);
    EXPECT_EQ(0.0, m_pEffectSlot->getMetaParameter());

    m_pEffectSlot->setMetaParameter(1.0, true);

    // Let enough time pass by to exceed soft-takeover's override interval.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() +
                             mixxx::Duration::fromMillis(2));

    // Ignored by SoftTakeover since it is too far from the current
    // metaknob value of 1.0.
    m_pChainSlot->setSuperParameter(0.5);
    EXPECT_EQ(1.0, m_pEffectSlot->getMetaParameter());
}

TEST_F(MetaLinkTest, SuperToMeta_Softtakeover_EffectDisabled) {
    // Soft takeover should not occur when the effect is disabled;
    // metaknob values should always jump to match the superknob.
    // Effects are disabled by default.
    m_pChainSlot->setSuperParameter(0.5);
    EXPECT_EQ(0.5, m_pEffectSlot->getMetaParameter());

    m_pEffectSlot->setMetaParameter(1.0, true);

    // Let enough time pass by to exceed soft-takeover's override interval.
    mixxx::Time::setTestElapsedTime(SoftTakeover::TestAccess::getTimeThreshold() +
                             mixxx::Duration::fromMillis(2));

    m_pChainSlot->setSuperParameter(0.0);
    EXPECT_EQ(0.0, m_pEffectSlot->getMetaParameter());
}

TEST_F(MetaLinkTest, HalfLinkTakeover) {
    // An effect that is linked to half of a knob should be more tolerant of
    // takeover changes.

    m_pEffectSlot->slotEnabled(1.0);

    // We have to recreate the effect because we want a neutral point at
    // 0 or 1.
    QString group = StandardEffectRack::formatEffectSlotGroupString(
        0, 0, 0);
    EffectManifestPointer pManifest(new EffectManifest());
    pManifest->setId("org.mixxx.test.effect2");
    pManifest->setName("Test Effect2");
    EffectManifestParameterPointer low = pManifest->addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter (neutral at 1.0)"));
    low->setControlHint(EffectManifestParameter::ControlHint::KNOB_LINEAR);
    low->setSemanticHint(EffectManifestParameter::SemanticHint::UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UnitsHint::UNKNOWN);
    low->setNeutralPointOnScale(1.0);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(1.0);
    registerTestEffect(pManifest, false);
    // Check the controls reflect the state of their loaded effect.
    EffectPointer pEffect = m_pEffectsManager->instantiateEffect(pManifest->id());
    m_pEffectSlot->loadEffect(pEffect, false);
    QString itemPrefix = EffectParameterSlot::formatItemPrefix(0);
    m_pControlValue.reset(new ControlProxy(group, itemPrefix));
    m_pControlLinkType.reset(new ControlProxy(group,
            itemPrefix + QString("_link_type")));
    m_pControlLinkInverse.reset(new ControlProxy(group,
            itemPrefix + QString("_link_inverse")));

    // OK now the actual test.
    // 1.5 is a bit of a magic number, but it's enough that a regular
    // linked control will fail the soft takeover test and not change the
    // value...
    double newParam = 0.5 - SoftTakeover::kDefaultTakeoverThreshold * 1.5;
    m_pEffectSlot->slotEffectMetaParameter(newParam);
    EXPECT_EQ(1.0, m_pControlValue->get());

    // ...but that value is still within the tolerance of a linked-left
    // and linked-right control.  So if we set the exact same newParam,
    // we should see the control change as expected.
    m_pEffectSlot->slotEffectMetaParameter(0.5);
    m_pControlValue->set(1.0);
    m_pControlLinkType->set(
        static_cast<double>(EffectManifestParameter::LinkType::LINKED_LEFT));
    m_pEffectSlot->syncSofttakeover();
    m_pEffectSlot->slotEffectMetaParameter(newParam);
    EXPECT_EQ(newParam * 2.0, m_pControlValue->get());

    // This tolerance change should also work for linked-right controls.
    m_pEffectSlot->slotEffectMetaParameter(0.5);
    m_pControlValue->set(1.0);
    m_pControlLinkType->set(
        static_cast<double>(EffectManifestParameter::LinkType::LINKED_RIGHT));
    m_pEffectSlot->syncSofttakeover();
    newParam = 0.5 + SoftTakeover::kDefaultTakeoverThreshold * 1.5;
    m_pEffectSlot->slotEffectMetaParameter(newParam);
    EXPECT_DOUBLE_EQ(0.0703125, m_pControlValue->get());
}
