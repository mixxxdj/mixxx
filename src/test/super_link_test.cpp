#include <gtest/gtest.h>

#include <QtDebug>
#include <QScopedPointer>

#include "mixxxtest.h"
#include "test/baseeffecttest.h"
#include "effects/effectparameterslot.h"
#include "effects/effect.h"

class SuperLinkTest : public BaseEffectTest {
  public:
    SuperLinkTest() {
    }

  protected:
    virtual void SetUp() {
        m_pEffectsManager->registerGroup("[Master]");
        m_pEffectsManager->registerGroup("[Headphone]");
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
