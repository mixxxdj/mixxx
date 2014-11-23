#ifndef STAT_H
#define STAT_H

#include <QMap>
#include <QVector>
#include <QString>

#include "util/experiment.h"

struct StatReport;

class Stat {
  public:
    enum StatType {
        UNSPECIFIED = 0,
        COUNTER,
        DURATION_MSEC,
        DURATION_NANOSEC,
        DURATION_SEC,
        EVENT,
        EVENT_START,
        EVENT_END,
    };

    static QString statTypeToString(StatType type) {
        switch (type) {
            case UNSPECIFIED:
                return "UNSPECIFIED";
            case COUNTER:
                return "COUNTER";
            case DURATION_MSEC:
                return "DURATION_MSEC";
            case DURATION_NANOSEC:
                return "DURATION_NANOSEC";
            case DURATION_SEC:
                return "DURATION_SEC";
            case EVENT:
                return "EVENT";
            case EVENT_START:
                return "START";
            case EVENT_END:
                return "END";
            default:
                return "UNKNOWN";
        }
    }

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
        // Used for marking stats recorded in EXPERIMENT mode.
        STATS_EXPERIMENT  = 0x0800,
        // Used for marking stats recorded in BASE mode.
        STATS_BASE        = 0x1000,
    };
    typedef int ComputeFlags;

    static Experiment::Mode modeFromFlags(ComputeFlags flags) {
        if (flags & Stat::STATS_EXPERIMENT) {
            return Experiment::EXPERIMENT;
        } else if (flags & Stat::STATS_BASE) {
            return Experiment::BASE;
        }
        return Experiment::OFF;
    }

    static ComputeFlags experimentFlags(ComputeFlags flags) {
        switch (Experiment::mode()) {
            case Experiment::EXPERIMENT:
                return flags | STATS_EXPERIMENT;
            case Experiment::BASE:
                return flags | STATS_BASE;
            default:
            case Experiment::OFF:
                return flags;
        }
    }

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
    qint64 time;
    Stat::StatType type;
    Stat::ComputeFlags compute;
    double value;
};

#endif /* STAT_H */
