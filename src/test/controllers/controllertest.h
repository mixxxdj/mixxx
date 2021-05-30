#pragma once

#include <QThread>
#include <QtDebug>

#include "controllers/controllerdebug.h"
#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
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
        m_pCEngine = new ControllerScriptEngineLegacy(nullptr);
        m_pCEngine->initialize();
        ControllerDebug::setEnabled(true);
        m_pRack = m_pEffectsManager->addStandardEffectRack();
        m_pQuickRack = m_pEffectsManager->addQuickEffectRack();
        m_pQuickRack->setupForGroup("[Channel1]");
        m_pQuickRack->setupForGroup("[Channel2]");
        m_pQuickRack->setupForGroup("[Channel3]");
        m_pQuickRack->setupForGroup("[Channel4]");
    }

    void TearDown() override {
        delete m_pCEngine;
        mixxx::Time::setTestMode(false);
    }

    bool evaluateScriptFile(const QFileInfo& scriptFile) {
        return m_pCEngine->evaluateScriptFile(scriptFile);
    }

    QJSValue evaluate(const QString& program) {
        return m_pCEngine->evaluateCodeString(program);
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

    ControllerScriptEngineLegacy* m_pCEngine;
    StandardEffectRackPointer m_pRack;
    QuickEffectRackPointer m_pQuickRack;
};
