#include "realtimehelper.h"
#include <QDebug>
#include <QMetaType>
#include <QThread>
//#include <#include "qdbusutil_p.h"QDbusUtil>
//#include <qdbusutil>

enum REALTIME_STATUS {
    UNCHECKED = -1,
    UNAVAILABLE = 0,
    HIGH_PRIO = 1,
    AVAILABLE = 100
};


#if __LINUX__
#include <QDBusInterface>

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <sched.h>
#include <sys/resource.h>

#include "util/rlimit.h"
#endif


namespace mixxx {

int RealtimeHelper::m_checked = REALTIME_STATUS::UNCHECKED;
quint32 RealtimeHelper::m_max_rt_prio = 0;
qint64 RealtimeHelper::m_max_rt_usec = 0;
qint32 RealtimeHelper::m_min_nice = 0;

void RealtimeHelperTestThread::run() {
    RealtimeHelper::m_checked = REALTIME_STATUS::UNAVAILABLE;

    if(RealtimeHelper::do_requestHighPriority(0, -2)) {
        RealtimeHelper::m_checked = REALTIME_STATUS::HIGH_PRIO;
    }

    if (RealtimeHelper::do_requestRealtime(0, 20, 10000)) {
        RealtimeHelper::m_checked = REALTIME_STATUS::AVAILABLE;
    }
}


bool RealtimeHelper::realtimeAvailable() {
    if (RealtimeHelper::m_checked == REALTIME_STATUS::UNCHECKED) {
        checkRealtime();
    }
    return RealtimeHelper::m_checked >= REALTIME_STATUS::AVAILABLE;
}

void RealtimeHelper::prepare() {
    // we have to ensure SCHED_RESET_ON_FORK is set, otherwise rtkit will not give us rt permissions
    struct sched_param sp;

    if (sched_getparam(0, &sp)) {
        qWarning() << "sched_getparam failed";
        RealtimeHelper::m_checked = REALTIME_STATUS::UNAVAILABLE;
        return;
    }

    int policy = SCHED_OTHER | SCHED_RESET_ON_FORK;
    if(sched_setscheduler(0, policy, &sp)) {
        qWarning() << "Error setting scheduler: " << errno;
    }
}

QVariant RealtimeHelper::getRTProperty(QString property) {
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.RealtimeKit1",
                                                          "/org/freedesktop/RealtimeKit1",
                                                          QLatin1String("org.freedesktop.DBus.Properties"),
                                                          QLatin1String("Get"));
    QList<QVariant> arguments;
    arguments << "org.freedesktop.RealtimeKit1" << property;
    message.setArguments(arguments);
    QDBusConnection connection = QDBusConnection::systemBus();
    QDBusMessage reply = connection.call(message);
    qDebug() << reply;

    return reply.arguments()[0].value<QDBusVariant>().variant();
}

bool RealtimeHelper::checkRealtime() {

#if __LINUX__

    qDebug() << "RLimit Cur " << RLimit::getCurRtPrio();
    qDebug() << "RLimit Max " << RLimit::getMaxRtPrio();

    // Even if for example rtkit is available, we can not be sure the upgrade path
    // will actually work. We could get a permission denied from policykit for example
    // we therefore simple spawn a test thread that tries to upgrade as much as possible
    // and record the sucess.

    RealtimeHelperTestThread *testThread = new RealtimeHelperTestThread();
    testThread->start();
    testThread->wait();
    qDebug() << "Finished realtime testing. Result: " << QString::number(RealtimeHelper::m_checked);

    // get the limits from realtimekit
    RealtimeHelper::m_max_rt_prio = getRTProperty(QString("MaxRealtimePriority")).toUInt();
    RealtimeHelper::m_min_nice = getRTProperty(QString("MinNiceLevel")).toInt();
    RealtimeHelper::m_max_rt_usec = getRTProperty(QString("RTTimeUSecMax")).toLongLong();
    qDebug() << "max_realtime_priority: " << RealtimeHelper::m_max_rt_prio << \
                " max_rttime:" << RealtimeHelper::m_max_rt_usec << \
                " min_nice_level:" << RealtimeHelper::m_min_nice;
#else // UNSUPPORTED PLATFORM

    RealtimeHelper::m_checked = REALTIME_STATUS::UNAVAILABLE;

#endif
    return RealtimeHelper::m_checked == REALTIME_STATUS::AVAILABLE;
}

bool RealtimeHelper::callRTKit(QString method, quint64 threadid, QVariant qprio) {
    if (threadid == 0) {
        threadid = syscall(SYS_gettid);
    }

    QVariant qtid = QVariant::fromValue<quint64>(threadid);
    //QVariant qprio = QVariant::fromValue((int)priority);

    QDBusInterface *iface = new QDBusInterface("org.freedesktop.RealtimeKit1", "/org/freedesktop/RealtimeKit1", "org.freedesktop.RealtimeKit1",
                               QDBusConnection::systemBus(), nullptr);
    if (!iface->isValid()) {
        return false;
    }

    QDBusMessage msg = iface->call(method, qtid, qprio);
    if (msg.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Error requesting realtime priority from org.freedesktop.RealtimeKit1: " << msg.errorMessage();
        return false;
    }
    return true;
}

bool RealtimeHelper::requestHighPriority(quint64 threadid, int priority) {
    if (RealtimeHelper::m_checked == REALTIME_STATUS::UNCHECKED) {
        checkRealtime();
    }
    if (RealtimeHelper::m_checked == REALTIME_STATUS::UNAVAILABLE) {
        return false;
    }
    return do_requestHighPriority(threadid, priority);
}

bool RealtimeHelper::requestRealtime(quint64 threadid, unsigned int priority, qint64 rlim_max) {
    if (RealtimeHelper::m_checked == REALTIME_STATUS::UNCHECKED) {
        checkRealtime();
    }
    if (RealtimeHelper::m_checked == REALTIME_STATUS::UNAVAILABLE) {
        return false;
    }
    return do_requestRealtime(threadid, priority, rlim_max);
}

QString RealtimeHelper::getRealtimeStatus() {
    if (RealtimeHelper::m_checked == REALTIME_STATUS::AVAILABLE) {
        return QObject::tr("Realtime scheduling is enabled.");
    } else if (RealtimeHelper::m_checked == REALTIME_STATUS::HIGH_PRIO) {
        return QObject::tr("Realtime scheduling is not availible. High priority is.");
    } else {
        return QObject::tr("No higher priorities are available.");
    }
}

// actual implementation. We seperate the public API because we use the implemenation in
// the test thread

#if __LINUX__

bool RealtimeHelper::do_requestHighPriority(quint64 threadid, int priority) {
    errno = 0;
    if(nice(priority) != -1 && errno == 0) {
        // nice actually worked for example though capabilities
        return true;
    }
    // If we request a priority smaller then minimum allowed the request will fail.
    // better we use the lowest possible value
    QVariant qprio = QVariant::fromValue((int)qMax(priority, RealtimeHelper::m_min_nice));
    return callRTKit("MakeThreadHighPriority", threadid, priority);
}


bool RealtimeHelper::do_requestRealtime(quint64 threadid, unsigned int priority, qint64 rlim_max) {
    // try to use setsched to get realtime priority
    struct sched_param p;
    memset(&p, 0, sizeof(p));
    p.sched_priority = priority;
    if (sched_setscheduler(0, SCHED_RR|SCHED_RESET_ON_FORK, &p) == 0) {
        return true;
    }

    // we check if the rlimt is within the allowed budget
    if ( rlim_max > RealtimeHelper::m_max_rt_usec ) {
        qWarning() << "Requested realtime budget not allowed by realtimekit. Requested: " << rlim_max << " allowed:" << RealtimeHelper::m_max_rt_usec;
        // we should stop here because there is no safe way to request that much budget. realtimekit will kill the process if the budget is overused
        return false;
    }
    // cap the requested priority to whats allowed
    if ( priority > RealtimeHelper::m_max_rt_prio ) {
        qWarning() << "Requested realtime priority" << priority << " capping to allowed:" << RealtimeHelper::m_max_rt_prio;
        priority = RealtimeHelper::m_max_rt_prio;
    }
    // for rtkit we need to set rlim_max
    struct rlimit rlim;

    memset(&rlim, 0, sizeof(rlim));
    rlim.rlim_cur = rlim.rlim_max = rlim_max; /* 200ms - default for rtkit */
    if ((setrlimit(RLIMIT_RTTIME, &rlim) < 0)) {
        fprintf(stderr, "Failed to set RLIMIT_RTTIME: %s\n", strerror(errno));
    }
    QVariant qprio = QVariant::fromValue((unsigned int)priority);
    return callRTKit("MakeThreadRealtime", threadid, qprio);
}

#else // UNSUPPORTED PLATFORM
bool RealtimeHelper::do_requestRealtime(quint64 threadid, unsigned int priority, qulonglong rlim_max) {
    return false;
}
bool RealtimeHelper::do_requestHighPriority(quint64 threadid, int priority) {
    return false;
}
#endif


}
