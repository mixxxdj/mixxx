#pragma once

#include <QThread>
#include <QtDebug>

#include "controllers/controllerdebug.h"
#include "controllers/controllerengine.h"
#include "test/mixxxtest.h"
#include "util/time.h"

class ControllerTest : public MixxxTest {
  protected:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::setTestElapsedTime(mixxx::Duration::fromMillis(10));
        QThread::currentThread()->setObjectName("Main");
        m_pCEngine = new ControllerEngine(nullptr, config());
        ControllerDebug::enable();
        m_pCEngine->setPopups(false);
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
};