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

inline
TrackCollectionManager* getTrackCollectionManager(
        TrackCollectionManager* trackCollectionManager) {
    if (trackCollectionManager) {
        // Ensure that the track collection manager is only accessed
        // from the same thread!
        VERIFY_OR_DEBUG_ASSERT(QThread::currentThread() == trackCollectionManager->thread()) {
            kLogger.critical()
                    << "Execution in different threads is not supported:"
                    << QThread::currentThread()
                    << "<>"
                    << trackCollectionManager->thread();
            return nullptr;
        }
    }
    return trackCollectionManager;
}

} // anonymous namespace

TrackLoader::TrackLoader(
        TrackCollectionManager* trackCollectionManager,
        QObject* parent)
    : QObject(parent),
      m_trackCollectionManager(trackCollectionManager) {
    std::call_once(registerMetaTypesOnceFlag, registerMetaTypes);
}

void TrackLoader::invokeSlotLoadTrack(
        TrackRef trackRef,
        Qt::ConnectionType connectionType) {
    DEBUG_ASSERT(connectionType == (connectionType & ~Qt::UniqueConnection));
    DEBUG_ASSERT((thread() == QThread::currentThread()) ||
            (connectionType != Qt::DirectConnection));
    QMetaObject::invokeMethod(
            this,
            "slotLoadTrack",
            connectionType,
            Q_ARG(TrackRef, std::move(trackRef)));
}

void TrackLoader::slotLoadTrack(
        TrackRef trackRef) {
    DEBUG_ASSERT(thread() == QThread::currentThread());
    TrackPointer trackPtr;
    const auto trackCollectionManager =
            getTrackCollectionManager(m_trackCollectionManager);
    if (trackCollectionManager) {
        trackPtr = trackCollectionManager->getOrAddTrack(trackRef);
    } else {
        kLogger.warning()
                << "Track collection manager not accessible";
    }
    emit trackLoaded(std::move(trackRef), std::move(trackPtr));
}

} // namespace mixxx
