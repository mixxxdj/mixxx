#include "library/trackloader.h"

#include <QMetaMethod>
#include <QThread>
#include <mutex>

#include "library/trackcollectionmanager.h"
#include "moc_trackloader.cpp"
#include "util/logger.h"

namespace mixxx {

namespace {

const Logger kLogger("TrackLoader");

std::once_flag registerMetaTypesOnceFlag;

void registerMetaTypes() {
    qRegisterMetaType<TrackId>();
    qRegisterMetaType<TrackRef>();
    qRegisterMetaType<TrackPointer>();
}

} // anonymous namespace

TrackLoader::TrackLoader(
        TrackCollectionManager* trackCollectionManager,
        QObject* parent)
    : QObject(parent),
      m_trackCollectionManager(trackCollectionManager) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypes);
    // Must be collocated with the TrackCollectionManager
    DEBUG_ASSERT(m_trackCollectionManager);
    moveToThread(m_trackCollectionManager->thread());
}

void TrackLoader::invokeSlotLoadTrack(
        const TrackRef& trackRef,
        Qt::ConnectionType connectionType) {
    DEBUG_ASSERT(connectionType == (connectionType & ~Qt::UniqueConnection));
    DEBUG_ASSERT((thread() == QThread::currentThread()) ||
            (connectionType != Qt::DirectConnection));
    QMetaObject::invokeMethod(
            this,
            [this, trackRef = std::move(trackRef)] {
                this->slotLoadTrack(trackRef);
            },
            connectionType);
}

void TrackLoader::slotLoadTrack(
        const TrackRef& trackRef) {
    VERIFY_OR_DEBUG_ASSERT(m_trackCollectionManager) {
        kLogger.warning()
                << "Track collection manager not accessible";
    }
    // Verify that still collocated with the TrackCollectionManager
    DEBUG_ASSERT(thread() == m_trackCollectionManager->thread());
    TrackPointer trackPtr = m_trackCollectionManager->getOrAddTrack(trackRef);
    emit trackLoaded(std::move(trackRef), std::move(trackPtr));
}

} // namespace mixxx
