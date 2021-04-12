#ifdef __LINUX__
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <QFile>
#endif

#include <QThread>
#include <QtDebug>

#include "util/cpupinning.h"

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

namespace mixxx {

#ifdef __LINUX__
bool CpuPinning::moveThreadToCpuset(const QString& cgroup) {
    qDebug() << "Move task" << gettid() << "to cpuset" << cgroup;
    QString filename = QStringLiteral("/sys/fs/cgroup/cpuset/") + cgroup + QStringLiteral("/tasks");
    auto tasksFile = QFile(filename);
    if (!tasksFile.exists()) {
        qWarning() << "cgroup cpuset:" << cgroup << "does not exist" << tasksFile.errorString();
        return false;
    }
    if (!tasksFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Error writing into cpuset cgroup" << tasksFile.errorString();
        return false;
    }
    QTextStream out(&tasksFile);
    out << QString::number(gettid());
    tasksFile.close();
    return true;
}
#endif

bool CpuPinning::pinThreadToCpu(qint32 cpuid) {
#ifdef __LINUX__
    qDebug() << "Set CPU affinity for Thread ID" << gettid();

    pthread_t thread = pthread_self();
    cpu_set_t cpuset;
    int status;
    // Set affinity mask to include CPUs 0 to 7
    qDebug() << "Set CPU affinity for Thread ID" << gettid();

    CPU_ZERO(&cpuset);
    CPU_SET(cpuid, &cpuset);

    status = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (status != 0) {
        qWarning() << "Error setting cpu affinity" << status;
        return false;
    }

    // Check the actual affinity mask assigned to the thread

    status = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (status != 0) {
        qWarning() << "Error getting cpu affinity" << status;
        return false;
    }

    qDebug() << "Set returned by pthread_getaffinity_np() contained:\n";
    for (int j = 0; j < CPU_SETSIZE; j++) {
        if (CPU_ISSET(j, &cpuset)) {
            qDebug() << "Use CPU " << j;
        }
    }
    return true;

#else
    qWarning() << "CPU pinning not implemented on this platform";
    return false;
#endif
}

} // namespace mixxx
