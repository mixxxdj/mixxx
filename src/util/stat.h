#ifndef STAT_H
#define STAT_H

#include <QMap>
#include <QVector>
#include <QString>

class StatReport;

class Stat {
  public:
    enum StatType {
        UNSPECIFIED = 0,
        COUNTER,
        DURATION_MSEC,
        DURATION_NANOSEC,
        DURATION_SEC,
        TRACE_START,
        TRACE_FINISH,
    };
    enum ComputeTypes {
        NONE            = 0x0000,
        // O(1) in time and space.
        COUNT           = 0x0001,
        // O(1) in time and space
        SUM             = 0x0002,
        // O(1) in time and space.
        AVERAGE         = 0x0004,
        // O(1) in time and space.
        SAMPLE_VARIANCE = 0x0008,
        SAMPLE_MEDIAN   = 0x0010,
        // O(1) in time and space.
        MIN             = 0x0020,
        // O(1) in time and space.
        MAX             = 0x0040,
        // O(1) in time, O(k) in space where k is # of distinct values.
        // Use carefully!
        HISTOGRAM       = 0x0080,
        // O(1) in time, O(n) in space where n is the # of reports.
        // Use carefully!
        VALUES          = 0x0100,
        // TODO, track average reports per second
        REPORTS_PER_SECOND = 0x0200,
        // TODO, track the time in between received reports.
        REPORT_TIME_DELTA = 0x0400,
    };
    typedef int ComputeFlags;

    explicit Stat();
    void processReport(const StatReport& report);
    QString valueUnits() const;

    double variance() const {
        return m_report_count > 1 ? m_variance_sk / (m_report_count - 1) : 0.0;
    }

    QString m_tag;
    StatType m_type;
    ComputeFlags m_compute;
    QVector<double> m_values;
    double m_report_count;
    double m_sum;
    double m_min;
    double m_max;
    double m_variance_mk;
    double m_variance_sk;
    QMap<double, double> m_histogram;

    static bool track(const QString& tag,
                      Stat::StatType type,
                      Stat::ComputeFlags compute,
                      double value);
};

QDebug operator<<(QDebug dbg, const Stat &stat);

struct StatReport {
    char* tag;
    Stat::StatType type;
    Stat::ComputeFlags compute;
    double value;
    // TODO(XXX): time?
};

#endif /* STAT_H */
