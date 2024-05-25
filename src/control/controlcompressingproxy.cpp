#include "control/controlcompressingproxy.h"

#include "moc_controlcompressingproxy.cpp"

namespace {
constexpr int kMaxNumOfRecursions = 128;
} // namespace

// Event queue compressing proxy
CompressingProxy::CompressingProxy(const ConfigKey& key,
        const RuntimeLoggingCategory& logger,
        QObject* pParent)
        : QObject(pParent),
          m_key(key),
          m_logger(logger),
          m_recursiveSearchForLastEventOngoing(false),
          m_recursionDepth(0) {
}

// This function is called recursive by QCoreApplication::sendPostedEvents, until no more events are in the queue.
// When the last event is found, QCoreApplication::sendPostedEvents returns for the processQueuedEvents() instance of this event,
// and m_recursiveSearchForLastEventOngoing is set to false, while returning true itself.
// All previous started instances of processQueuedEvents() will return false in consequence,
// because the return value depends on the member variable and not on the stack of the instance.
CompressingProxy::StateOfProcessQueuedEvent CompressingProxy::processQueuedEvents() {
    m_recursiveSearchForLastEventOngoing = true;

    if (m_recursionDepth >= kMaxNumOfRecursions) {
        // To many events in queue -> Delete all unprocessed events in queue, to prevent stack overflow
        QCoreApplication::removePostedEvents(this, QEvent::MetaCall);

        qCWarning(m_logger) << "Deleted unprocessed events for " + m_key.group +
                        ", " + m_key.item +
                        ", because too many events were in the queue. This "
                        "indicates a serious performance problem with the "
                        "controller mapping.";

        // We just return, without resetting m_recursiveSearchForLastEventOngoing,
        // this ensures that the event found in the last iteration will be send
        return StateOfProcessQueuedEvent::NoEvent;
    }

    m_recursionDepth++;
    // sendPostedEvents recursive executes slotValueChanged until no more events for this slot are in the queue
    // Each call of QCoreApplication::sendPostedEvents triggers the processing of the next event in the queue,
    // by sending the signal to execute slotValueChanged again
    QCoreApplication::sendPostedEvents(this, QEvent::MetaCall);

    if (m_recursiveSearchForLastEventOngoing && m_recursionDepth > 1) {
        qCWarning(m_logger)
                << "Skipped" << (m_recursionDepth - 1)
                << "superseded events from " + m_key.group + ", " + m_key.item +
                        ", because the controller mapping couldn't process "
                        "them fast enough.";
    }

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
