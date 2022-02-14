#include "control/grouphandle.h"

#include <QAtomicInt>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>

namespace {

QMutex allGroupHandlesByNameMutex;

typedef QHash<QString, GroupHandle> AllGroupHandlesByName;
AllGroupHandlesByName allGroupHandlesByName;

QAtomicInt allGroupHandlesFrozen;

} // namespace

GroupHandle getOrCreateGroupHandleByName(const QString& name, bool create) {
    DEBUG_ASSERT(name == name.trimmed());
    if (allGroupHandlesFrozen.loadRelaxed()) {
        // lock-free
        DEBUG_ASSERT(!create);
        auto iter = allGroupHandlesByName.find(name);
        if (iter != allGroupHandlesByName.end()) {
            return iter.value();
        }
    } else {
        // blocking
        auto locked = QMutexLocker(&allGroupHandlesByNameMutex);
        auto iter = allGroupHandlesByName.find(name);
        if (iter != allGroupHandlesByName.end()) {
            // Unlock mutex before logging (I/O)
            locked.unlock();
            qDebug() << "Found existing group handle" << iter.value();
            return iter.value();
        }
        if (create && !allGroupHandlesFrozen.loadAcquire()) {
            const auto index = allGroupHandlesByName.size();
            // This will leak memory, i.e. descriptors will never be
            // freed during a session until application shutdown.
            const auto handle = new mixxx::grouphandle_private::Descriptor{index, name};
            allGroupHandlesByName.insert(name, handle);
            // Unlock mutex before logging (I/O)
            locked.unlock();
            qInfo() << "Created new group handle" << handle;
            return handle;
        }
    }
    qWarning() << "Unknown group name" << name;
    return kNullGroupHandle;
}

void freezeAllGroupHandles() {
    const auto locked = QMutexLocker(&allGroupHandlesByNameMutex);
    allGroupHandlesByName.squeeze();
    allGroupHandlesFrozen.storeRelease(1);
}

int resetAllGroupHandles() {
    auto locked = QMutexLocker(&allGroupHandlesByNameMutex);
    AllGroupHandlesByName tmpGroupHandlesByName;
    tmpGroupHandlesByName.swap(allGroupHandlesByName);
    allGroupHandlesFrozen.storeRelease(0);
    locked.unlock();
    for (const auto groupHandle : qAsConst(tmpGroupHandlesByName)) {
        qInfo() << "Deleting" << groupHandle;
        delete groupHandle;
    }
    return tmpGroupHandlesByName.size();
}

namespace mixxx {
namespace grouphandle_private {

QDebug operator<<(QDebug dbg, const mixxx::grouphandle_private::Descriptor& arg) {
    return dbg << "GroupHandle{" << arg.m_index << arg.m_name << "}";
}

} // namespace grouphandle_private
} // namespace mixxx
