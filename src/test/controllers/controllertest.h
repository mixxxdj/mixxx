#pragma once

#include <QThread>
#include <QtDebug>

#include "controllers/controllerdebug.h"
#include "controllers/controllerengine.h"
#include "effects/effectrack.h"
#include "test/signalpathtest.h"
#include "util/time.h"

// ControllerTest inherits from BaseSignalPathTest so that all of the standard
// channels, effects units, etc exist.
class ControllerTest : public BaseSignalPathTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        QThread::currentThread()->setObjectName("Main");
        m_pCEngine = new ControllerEngine(nullptr, config());
        ControllerDebug::enable();
        m_pCEngine->setPopups(false);
        m_pRack = m_pEffectsManager->addStandardEffectRack();
        // EffectChainSlotPointer pChainSlot = pRack->getEffectChainSlot(iChainNumber);
        // pChainSlot->registerInputChannel(m_master);
        m_pQuickRack = m_pEffectsManager->addQuickEffectRack();
        // Only 3 decks in BaseSignalPathTest
        m_pQuickRack->setupForGroup("[Channel1]");
        m_pQuickRack->setupForGroup("[Channel2]");
        m_pQuickRack->setupForGroup("[Channel3]");
        m_pQuickRack->setupForGroup("[Channel4]");
        // pChainSlot = pRack->getEffectChainSlot(iChainNumber);
        // pChainSlot->registerInputChannel(m_master);
    }

    void TearDown() override {
        m_pCEngine->gracefulShutdown();
        delete m_pCEngine;
        mixxx::Time::setTestMode(false);
    }

    QScriptValue evaluate(const QString& program) {
        QScriptValue ret;
        EXPECT_TRUE(m_pCEngine->evaluateWithReturn(program, QString(), &ret));
        return ret;
    }

    void processEvents() {
        // QCoreApplication::processEvents() only processes events that were
        // queued when the method was called. Hence, all subsequent events that
        // are emitted while processing those queued events will not be
        // processed and are enqueued for the next event processing cycle.
        // Calling processEvents() twice ensures that at least all queued and
        // the next round of emitted events are processed.
        application()->processEvents();
        application()->processEvents();
    }

    ControllerEngine* m_pCEngine;
    StandardEffectRackPointer m_pRack;
    QuickEffectRackPointer m_pQuickRack;
};