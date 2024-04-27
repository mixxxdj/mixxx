#include "widget/wnumberduration.h"

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
        m_displayFormat = TrackTime::DisplayFormat::TRADITIONAL;
    }
}

// Reimplementing WNumber::setValue
void WNumberDuration::setValue(double dSeconds) {
    QString (*timeFormat)(double dSeconds, mixxx::Duration::Precision precision);

    if (m_displayFormat == TrackTime::DisplayFormat::KILO_SECONDS) {
        timeFormat = &mixxx::Duration::formatKiloSeconds;
    } else if (m_displayFormat == TrackTime::DisplayFormat::SECONDS_LONG) {
        timeFormat = &mixxx::Duration::formatSecondsLong;
    } else if (m_displayFormat == TrackTime::DisplayFormat::SECONDS) {
        timeFormat = &mixxx::Duration::formatSeconds;
    } else {
        timeFormat = &mixxx::Duration::formatTime;
    }

    mixxx::Duration::Precision precision;
    if (m_displayFormat != TrackTime::DisplayFormat::TRADITIONAL_COARSE) {
        precision = mixxx::Duration::Precision::CENTISECONDS;
    } else {
        precision = mixxx::Duration::Precision::SECONDS;
    }

    QString formattedValue;
    if (dSeconds >= 0.0) {
        formattedValue = timeFormat(dSeconds, precision);
    } else {
        formattedValue = QLatin1String("-") % timeFormat(-dSeconds, precision);
    }

    if (m_skinText.contains("%1")) {
        setText(m_skinText.arg(formattedValue));
    } else {
        setText(m_skinText + formattedValue);
    }
}
