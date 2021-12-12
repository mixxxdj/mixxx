#include "control/controlcompressingproxy.h"

#include "moc_controlcompressingproxy.cpp"

// Event queue compressing proxy
CompressingProxy::CompressingProxy(QObject* parent)
        : QObject(parent) {
}

// This function is called recursive by QCoreApplication::sendPostedEvents, until no more events are in the queue.
// When the last event is found, QCoreApplication::sendPostedEvents returns for the isLatestEventInQueue() instance of this event,
// and m_recursiveSearchForLastEventOngoing is set to false, while returning true itself.
// All previous started instances of isLatestEventInQueue() will return false in consequence,
// because the return value depends on the member variable and not on the stack of the instance.
bool CompressingProxy::isLatestEventInQueue() {
    m_recursiveSearchForLastEventOngoing = true;

    // sendPostedEvents recursive executes slotValueChanged until no more events for this slot are in the queue
    // Each call of sendPostedEvents triggers the processing of the next event in the queue,
    // by sending the signal to execute slotValueChanged again
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    // Execution continues here, when last event for this slot is processed
    bool isLastEvent = m_recursiveSearchForLastEventOngoing;
    m_recursiveSearchForLastEventOngoing = false;
    return isLastEvent;
}

void CompressingProxy::slotValueChanged(double value, QObject* obj) {
    if (isLatestEventInQueue()) {
        emit signalValueChanged(value, obj);
    }
}
