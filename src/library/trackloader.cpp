#include "library/trackloader.h"

#include <QMetaMethod>
#include <QThread>

#include <mutex>

#include "library/trackcollectionmanager.h"
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
        TrackRef trackRef,
        Qt::ConnectionType connectionType) {
    DEBUG_ASSERT(connectionType == (connectionType & ~Qt::UniqueConnection));
    DEBUG_ASSERT((thread() == QThread::currentThread()) ||
            (connectionType != Qt::DirectConnection));
    QMetaObject::invokeMethod(
            this,
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            "slotLoadTrack"
#else
            [this, trackRef = std::move(trackRef)] {
                this->slotLoadTrack(trackRef);
            }
#endif
            , connectionType
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
            , Q_ARG(TrackRef, std::move(trackRef))
#endif
            );
}

void TrackLoader::slotLoadTrack(
        TrackRef trackRef) {
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
