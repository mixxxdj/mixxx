#include <limits>
#include <cmath>

#include <QStringList>
#include <QtDebug>

#include "util/stat.h"
#include "util/statsmanager.h"

Stat::Stat()
        : m_type(UNSPECIFIED),
          m_compute(NONE),
          m_report_count(0),
          m_sum(0),
          m_min(std::numeric_limits<double>::max()),
          m_max(std::numeric_limits<double>::min()),
          m_variance_mk(0),
          m_variance_sk(0) {
}

QString Stat::valueUnits() const {
    switch (m_type) {
        case DURATION_MSEC:
            return "ms";
        case DURATION_NANOSEC:
            return "ns";
        case DURATION_SEC:
            return "s";
        case TRACE_START:
        case TRACE_FINISH:
        case UNSPECIFIED:
        default:
            return "";
    }
}

void Stat::processReport(const StatReport& report) {
    m_report_count++;
    if (m_compute & (Stat::SUM | Stat::AVERAGE)) {
        m_sum += report.value;
    }
    if (m_compute & Stat::MAX && report.value > m_max) {
        m_max = report.value;
    }
    if (m_compute & Stat::MIN && report.value < m_min) {
        m_min = report.value;
    }

    // Method comes from Knuth, see:
    // http://www.johndcook.com/standard_deviation.html
    if (m_compute & Stat::SAMPLE_VARIANCE) {
        if (m_report_count == 0.0) {
            m_variance_mk = report.value;
            m_variance_sk = 0.0;
        } else {
            double variance_mk_prev = m_variance_mk;
            m_variance_mk += (report.value - m_variance_mk) / m_report_count;
            m_variance_sk += (report.value - variance_mk_prev) * (report.value - m_variance_mk);
        }
    }

    if (m_compute & Stat::HISTOGRAM) {
        m_histogram[report.value] += 1.0;
    }

    if (m_compute & Stat::VALUES) {
        m_values.push_back(report.value);
    }
}

QDebug operator<<(QDebug dbg, const Stat &stat) {
    QStringList stats;
    if (stat.m_compute & Stat::COUNT) {
        stats << "count=" + QString::number(stat.m_report_count);
    }

    if (stat.m_compute & Stat::SUM) {
        stats << "sum=" + QString::number(stat.m_sum) + stat.valueUnits();
    }

    if (stat.m_compute & Stat::AVERAGE) {
        QString value = "average=";
        if (stat.m_report_count > 0) {
            value += QString::number(stat.m_sum / stat.m_report_count) + stat.valueUnits();
        } else {
            value += "XXX";
        }
        stats << value;
    }

    if (stat.m_compute & Stat::MIN) {
        QString value = "min=";
        if (stat.m_report_count > 0) {
            value += QString::number(stat.m_min) + stat.valueUnits();
        } else {
            value += "XXX";
        }
        stats << value;
    }

    if (stat.m_compute & Stat::MAX) {
        QString value = "max=";
        if (stat.m_report_count > 0) {
            value += QString::number(stat.m_max) + stat.valueUnits();
        } else {
            value += "XXX";
        }
        stats << value;
    }

    if (stat.m_compute & Stat::SAMPLE_VARIANCE) {
        double variance = stat.variance();
        stats << "variance=" + QString::number(variance) + stat.valueUnits() + "^2";
        if (variance >= 0.0) {
            stats << "stddev=" + QString::number(sqrt(variance)) + stat.valueUnits();
        }
    }

    if (stat.m_compute & Stat::SAMPLE_MEDIAN) {
        // TODO(rryan): implement
    }

    if (stat.m_compute & Stat::HISTOGRAM) {
        QStringList histogram;
        for (QMap<double, double>::const_iterator it = stat.m_histogram.begin();
             it != stat.m_histogram.end(); ++it) {
            histogram << QString::number(it.key()) + stat.valueUnits() + ":" +
                    QString::number(it.value());
        }
        stats << "histogram=" + histogram.join(",");
    }

    dbg.nospace() << "Stat(" << stat.m_tag << "," << stats.join(",") << ")";
    return dbg.maybeSpace();
}

// static
bool Stat::track(const QString& tag,
                 Stat::StatType type,
                 Stat::ComputeFlags compute,
                 double value) {
    if (!StatsManager::s_bStatsManagerEnabled) {
        return false;
    }
    StatReport report;
    report.tag = strdup(tag.toAscii().constData());
    report.type = type;
    report.compute = compute;
    report.value = value;
    StatsManager* pManager = StatsManager::instance();
    return pManager && pManager->maybeWriteReport(report);
}
