#include "control/controlcompressingproxy.h"

#include "moc_controlcompressingproxy.cpp"

// Event queue compressing proxy
CompressingProxy::CompressingProxy(QObject* parent)
        : QObject(parent), m_recursionDepth(0) {
}

// This function is called recursive by QCoreApplication::sendPostedEvents, until no more events are in the queue.
// When the last event is found, QCoreApplication::sendPostedEvents returns for the processQueuedEvents() instance of this event,
// and m_recursiveSearchForLastEventOngoing is set to false, while returning true itself.
// All previous started instances of processQueuedEvents() will return false in consequence,
// because the return value depends on the member variable and not on the stack of the instance.
StateOfProcessQueuedEvent CompressingProxy::processQueuedEvents() {
    m_recursiveSearchForLastEventOngoing = true;

    if (m_recursionDepth >= kMaxNumOfRecursions) {
        // To many events in queue -> Delete all unprocessed events in queue, to prevent stack overflow
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
        // We just return, without resetting m_recursiveSearchForLastEventOngoing,
        // this ensures that the event found in the last iteration will be send
        return StateOfProcessQueuedEvent::NoEvent;
    }

    m_recursionDepth++;
    // sendPostedEvents recursive executes slotValueChanged until no more events for this slot are in the queue
    // Each call of QCoreApplication::sendPostedEvents triggers the processing of the next event in the queue,
    // by sending the signal to execute slotValueChanged again
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);
    m_recursionDepth--;

    // Execution continues here, when last event for this slot is processed

    StateOfProcessQueuedEvent returnValue;
    if (m_recursiveSearchForLastEventOngoing) {
        returnValue = StateOfProcessQueuedEvent::LastEvent;
    } else {
        returnValue = StateOfProcessQueuedEvent::OutdatedEvent;
    }
    m_recursiveSearchForLastEventOngoing = false;
    return returnValue;
}

void CompressingProxy::slotValueChanged(double value, QObject* obj) {
    if (processQueuedEvents() == StateOfProcessQueuedEvent::LastEvent) {
        emit signalValueChanged(value, obj);
    }
}
