#include "widget/wnumberduration.h"

#include <QTime>

#include "control/controlproxy.h"
#include "moc_wnumberduration.cpp"
#include "util/duration.h"

WNumberDuration::WNumberDuration(QWidget* parent)
        : WNumber(parent),
          m_displayFormat(TrackTime::DisplayFormat::TRADITIONAL) {
}

void WNumberDuration::setup(const QDomNode& node, const SkinContext& context) {
    WNumber::setup(node, context);

    QString formatString = context.selectString(node, "TimeFormat");
    if (formatString == "kilo_seconds") {
        m_displayFormat = TrackTime::DisplayFormat::KILO_SECONDS;
    } else if (formatString == "seconds_long") {
        m_displayFormat = TrackTime::DisplayFormat::SECONDS_LONG;
    } else if (formatString == "seconds") {
        m_displayFormat = TrackTime::DisplayFormat::SECONDS;
    } else if (formatString == "traditional") {
        m_displayFormat = TrackTime::DisplayFormat::TRADITIONAL;
    } else if (formatString == "traditional_coarse") {
        m_displayFormat = TrackTime::DisplayFormat::TRADITIONAL_COARSE;
    } else {
        m_displayFormatString = formatString;
    }
}

// Reimplementing WNumber::setValue
void WNumberDuration::setValue(double dSeconds) {
    const double dSecondsAbs = abs(dSeconds);

    QString fmt;
    if (!m_displayFormatString.isNull() && !m_displayFormatString.isEmpty()) {
        // A more detailed format string was specified,
        // pass it on to QTime for formatting

        // NOTE: QTime() constructs a 'null' object,
        // but we need 'zero' here.
        QTime t = QTime(0, 0).addMSecs(static_cast<int>(
                dSeconds * mixxx::Duration::kMillisPerSecond));

        fmt = t.toString(m_displayFormatString);
    } else if (m_displayFormat == TrackTime::DisplayFormat::KILO_SECONDS) {
        fmt = mixxx::Duration::formatKiloSeconds(dSecondsAbs);
    } else if (m_displayFormat == TrackTime::DisplayFormat::SECONDS_LONG) {
        fmt = mixxx::Duration::formatSecondsLong(dSecondsAbs);
    } else if (m_displayFormat == TrackTime::DisplayFormat::SECONDS) {
        fmt = mixxx::Duration::formatSeconds(dSecondsAbs);
    } else if (m_displayFormat == TrackTime::DisplayFormat::TRADITIONAL_COARSE) {
        fmt = mixxx::Duration::formatTime(
                dSecondsAbs, mixxx::Duration::Precision::CENTISECONDS);
    } else {
        fmt = mixxx::Duration::formatTime(dSecondsAbs);
    }

    // Prepend "-" for negative durations
    if (dSeconds < 0.0) {
        fmt = QLatin1String("-") % fmt;
    }

    if (m_skinText.contains("%1")) {
        setText(m_skinText.arg(fmt));
    } else {
        setText(m_skinText + fmt);
    }
}
