#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "controllers/softtakeover.h"
#include "effects/effectparameterslot.h"
#include "effects/effect.h"
#include "mixxxtest.h"
#include "test/baseeffecttest.h"

class SuperLinkTest : public BaseEffectTest {
  protected:
    SuperLinkTest()
            : m_master(m_factory.getOrCreateHandle("[Master]"), "[Master]"),
              m_headphone(m_factory.getOrCreateHandle("[Headphone]"), "[Headphone]") {
        m_pEffectsManager->registerChannel(m_master);
        m_pEffectsManager->registerChannel(m_headphone);
        registerTestBackend();

        EffectChainPointer pChain(new EffectChain(m_pEffectsManager.data(),
                                                  "org.mixxx.test.chain1"));
        int iRackNumber = 0;
        int iChainNumber = 0;
        int iEffectNumber = 0;

        StandardEffectRackPointer pRack = m_pEffectsManager->addStandardEffectRack();
        EffectChainSlotPointer pChainSlot = pRack->addEffectChainSlot();
        // StandardEffectRack::addEffectChainSlot automatically adds 4 effect
        // slots. In the future we will probably remove this so this will just
        // start segfaulting.
        m_pEffectSlot = pChainSlot->getEffectSlot(0);

        QString group = StandardEffectRack::formatEffectSlotGroupString(
            iRackNumber, iChainNumber, iEffectNumber);

        EffectManifest manifest;
        manifest.setId("org.mixxx.test.effect");
        manifest.setName("Test Effect");

        EffectManifestParameter* low = manifest.addParameter();
        low->setId("low");
        low->setName(QObject::tr("Low"));
        low->setDescription(QObject::tr("Gain for Low Filter"));
        low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
        low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
        low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
        low->setNeutralPointOnScale(0.25);
        low->setDefault(1.0);
        low->setMinimum(0);
        low->setMaximum(1.0);

        registerTestEffect(manifest, false);

        // Check the controls reflect the state of their loaded effect.
        EffectPointer pEffect = m_pEffectsManager->instantiateEffect(manifest.id());

        m_pEffectSlot->loadEffect(pEffect);

        QString itemPrefix = EffectParameterSlot::formatItemPrefix(0);

        m_pControlValue.reset(new ControlObjectThread(group, itemPrefix));

        m_pControlLinkType.reset(new ControlObjectThread(group,
                itemPrefix + QString("_link_type")));

        m_pControlLinkInverse.reset(new ControlObjectThread(group,
                itemPrefix + QString("_link_inverse")));
    }

    ChannelHandleFactory m_factory;
    ChannelHandleAndGroup m_master;
    ChannelHandleAndGroup m_headphone;

    EffectSlotPointer m_pEffectSlot;
    QScopedPointer<ControlObjectThread> m_pControlValue;
    QScopedPointer<ControlObjectThread> m_pControlLinkType;
    QScopedPointer<ControlObjectThread> m_pControlLinkInverse;
};

TEST_F(SuperLinkTest, LinkDefault) {
    // default is not Linked, value must be unchanged
    m_pEffectSlot->syncSofttakeover();
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->onChainSuperParameterChanged(1.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->onChainSuperParameterChanged(0.5);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->onChainSuperParameterChanged(0.3);
    EXPECT_EQ(1.0, m_pControlValue->get());
}

TEST_F(SuperLinkTest, LinkLinked) {
    m_pEffectSlot->syncSofttakeover();
    m_pControlLinkType->set(EffectManifestParameter::LINK_LINKED);
    m_pEffectSlot->onChainSuperParameterChanged(1.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->onChainSuperParameterChanged(0.5);
    EXPECT_EQ(0.25, m_pControlValue->get());
}

TEST_F(SuperLinkTest, LinkLinkedInverse) {
    m_pEffectSlot->syncSofttakeover();
    m_pControlLinkType->set(EffectManifestParameter::LINK_LINKED);
    m_pControlLinkInverse->set(1.0);
    m_pEffectSlot->onChainSuperParameterChanged(0.0);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pEffectSlot->onChainSuperParameterChanged(0.5);
    EXPECT_EQ(0.25, m_pControlValue->get());
}

TEST_F(SuperLinkTest, Softtakeover) {
    m_pControlLinkType->set(EffectManifestParameter::LINK_LINKED);
    m_pEffectSlot->onChainSuperParameterChanged(0.5);
    EXPECT_EQ(1.0, m_pControlValue->get());
    m_pControlValue->set(0.1);
    m_pEffectSlot->onChainSuperParameterChanged(0.7);
    EXPECT_EQ(0.1, m_pControlValue->get());
}

TEST_F(SuperLinkTest, HalfLinkTakeover) {
    // An effect that is linked to half of a knob should be more tolerant of
    // takeover changes.

    // We have to recreate the effect because we want a neutral point at
    // 0 or 1.
    QString group = StandardEffectRack::formatEffectSlotGroupString(
        0, 0, 0);
    EffectManifest manifest;
    manifest.setId("org.mixxx.test.effect2");
    manifest.setName("Test Effect2");
    EffectManifestParameter* low = manifest.addParameter();
    low->setId("low");
    low->setName(QObject::tr("Low"));
    low->setDescription(QObject::tr("Gain for Low Filter (neutral at 1.0)"));
    low->setControlHint(EffectManifestParameter::CONTROL_KNOB_LINEAR);
    low->setSemanticHint(EffectManifestParameter::SEMANTIC_UNKNOWN);
    low->setUnitsHint(EffectManifestParameter::UNITS_UNKNOWN);
    low->setNeutralPointOnScale(1.0);
    low->setDefault(1.0);
    low->setMinimum(0);
    low->setMaximum(1.0);
    registerTestEffect(manifest, false);
    // Check the controls reflect the state of their loaded effect.
    EffectPointer pEffect = m_pEffectsManager->instantiateEffect(manifest.id());
    m_pEffectSlot->loadEffect(pEffect);
    QString itemPrefix = EffectParameterSlot::formatItemPrefix(0);
    m_pControlValue.reset(new ControlObjectThread(group, itemPrefix));
    m_pControlLinkType.reset(new ControlObjectThread(group,
            itemPrefix + QString("_link_type")));
    m_pControlLinkInverse.reset(new ControlObjectThread(group,
            itemPrefix + QString("_link_inverse")));

    // OK now the actual test.
    // 1.5 is a bit of a magic number, but it's enough that a regular
    // linked control will fail the soft takeover test and not change the
    // value...
    double newParam = 0.5 - SoftTakeover::kDefaultTakeoverThreshold * 1.5;
    m_pEffectSlot->onChainSuperParameterChanged(newParam);
    EXPECT_EQ(1.0, m_pControlValue->get());

    // ...but that value is still within the tolerance of a linked-left
    // and linked-right control.  So if we set the exact same newParam,
    // we should see the control change as expected.
    m_pEffectSlot->onChainSuperParameterChanged(0.5);
    m_pControlValue->set(1.0);
    m_pControlLinkType->set(EffectManifestParameter::LINK_LINKED_LEFT);
    m_pEffectSlot->syncSofttakeover();
    m_pEffectSlot->onChainSuperParameterChanged(newParam);
    EXPECT_EQ(newParam * 2.0, m_pControlValue->get());

    // This tolerance change should also work for linked-right controls.
    m_pEffectSlot->onChainSuperParameterChanged(0.5);
    m_pControlValue->set(1.0);
    m_pControlLinkType->set(EffectManifestParameter::LINK_LINKED_RIGHT);
    m_pEffectSlot->syncSofttakeover();
    newParam = 0.5 + SoftTakeover::kDefaultTakeoverThreshold * 1.5;
    m_pEffectSlot->onChainSuperParameterChanged(newParam);
    EXPECT_FLOAT_EQ(0.9296875, m_pControlValue->get());
}
