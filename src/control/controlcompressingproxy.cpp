#include "control/controlcompressingproxy.h"

#include "moc_controlcompressingproxy.cpp"

// Compressor
CompressingProxy::CompressingProxy(QObject* parent)
        : QObject(parent){};

bool CompressingProxy::emitCheck() {
    m_lastEventRecursionOngoing = true;

    // sendPostedEventstriggers recursive execution of slotValueChanged until no more events for this slot are in the queue
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    // Execution continues here, if last event for this slot is processed
    bool isLastEvent = m_lastEventRecursionOngoing;
    m_lastEventRecursionOngoing = false;
    return isLastEvent;
}

void CompressingProxy::slotValueChanged(double value, QObject* obj) {
    if (emitCheck())
        emit signalValueChanged(value, obj);
}
