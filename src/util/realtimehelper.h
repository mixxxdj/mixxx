#ifndef REALTIMEHELPER_H
#define REALTIMEHELPER_H

#include <QThread>
#include <QVariant>
#include <QString>


namespace mixxx {

class RealtimeHelperTestThread : public QThread {
    Q_OBJECT
    void run();
};

#define RLIMIT_DEFAULT 200000LL /* 200ms - default for rtkit */

// Code related to requesting priority changes/realtime priority
class RealtimeHelper
{
public:
    static bool realtimeAvailable();
    static void prepare();
    static bool checkRealtime();
    static QString getRealtimeStatus();
    static bool requestRealtime(quint64 threadid, unsigned int priority, qint64 rlim_max = RLIMIT_DEFAULT);
    static bool requestHighPriority(quint64 threadid, int priority);
protected:
    static QVariant getRTProperty(QString property);
    static bool do_requestRealtime(quint64 threadid, unsigned int priority, qint64 rlim_max);
    static bool do_requestHighPriority(quint64 threadid, int priority);
    static bool callRTKit(QString method, quint64 threadid, QVariant qprio);
    static int m_checked;
    static quint32 m_max_rt_prio;
    static qint64 m_max_rt_usec;
    static qint32 m_min_nice;



friend class RealtimeHelperTestThread;
};
}

#endif // REALTIMEHELPER_H
