// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QStringBuilder>

#include "widget/wnumberpos.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"
#include "util/duration.h"

WNumberPos::WNumberPos(const char* group, QWidget* parent)
        : WNumber(parent),
          m_dOldTimeElapsed(0.0) {
    m_pTimeElapsed = new ControlProxy(group, "time_elapsed", this);
    m_pTimeElapsed->connectValueChanged(SLOT(slotSetTimeElapsed(double)));
    m_pTimeRemaining = new ControlProxy(group, "time_remaining", this);
    m_pTimeRemaining->connectValueChanged(SLOT(slotTimeRemainingUpdated(double)));

    m_pShowTrackTimeRemaining = new ControlProxy(
            "[Controls]", "ShowDurationRemaining", this);
    m_pShowTrackTimeRemaining->connectValueChanged(
            SLOT(slotSetDisplayMode(double)));
    slotSetDisplayMode(m_pShowTrackTimeRemaining->get());

    m_pTimeFormat = new ControlProxy(
            "[Controls]", "TimeFormat", this);
    m_pTimeFormat->connectValueChanged(
            SLOT(slotSetTimeFormat(double)));
    slotSetTimeFormat(m_pTimeFormat->get());

}

void WNumberPos::mousePressEvent(QMouseEvent* pEvent) {
    bool leftClick = pEvent->buttons() & Qt::LeftButton;

    if (leftClick) {
        // Cycle through display modes
        if (m_displayMode == TrackTime::DisplayMode::Elapsed) {
            m_displayMode = TrackTime::DisplayMode::Remaining;
        } else if (m_displayMode == TrackTime::DisplayMode::Remaining) {
            m_displayMode = TrackTime::DisplayMode::ElapsedAndRemaining;
        } else if (m_displayMode == TrackTime::DisplayMode::ElapsedAndRemaining) {
            m_displayMode = TrackTime::DisplayMode::Elapsed;
        }

        m_pShowTrackTimeRemaining->set(static_cast<double>(m_displayMode));
        slotSetTimeElapsed(m_dOldTimeElapsed);
    }
}

// Reimplementing WNumber::setValue
void WNumberPos::setValue(double dValue) {
    // Ignore midi-scaled signals from the skin connection.
    Q_UNUSED(dValue);
    // Update our value with the old value.
    slotSetTimeElapsed(m_dOldTimeElapsed);
}

void WNumberPos::slotSetTimeElapsed(double dTimeElapsed) {
    double dTimeRemaining = m_pTimeRemaining->get();
    QString (*timeFormat)(double dSeconds, mixxx::Duration::Precision precision);

    if (m_displayFormat == TrackTime::DisplayFormat::KILO_SECOND) {
        timeFormat = &mixxx::Duration::formatKiloSeconds;
    } else if (m_displayFormat == TrackTime::DisplayFormat::HECTO_SECOND) {
        timeFormat = &mixxx::Duration::formatHectoSeconds;
    } else if (m_displayFormat == TrackTime::DisplayFormat::SECOND) {
       timeFormat = &mixxx::Duration::formatSeconds;
    } else {
        timeFormat = &mixxx::Duration::formatTime;
    }

    if (m_displayMode == TrackTime::DisplayMode::Elapsed) {
        if (dTimeElapsed >= 0.0) {
            setText(timeFormat(
                    dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS));
        } else {
            setText(QLatin1String("-") % timeFormat(
                    -dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS));
        }
    } else if (m_displayMode == TrackTime::DisplayMode::Remaining) {
        setText(QLatin1String("-") % timeFormat(
                dTimeRemaining, mixxx::Duration::Precision::CENTISECONDS));
    } else if (m_displayMode == TrackTime::DisplayMode::ElapsedAndRemaining) {
        if (dTimeElapsed >= 0.0) {
            setText(timeFormat(
                    dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS)
                    % QLatin1String("  -") % timeFormat(
                    dTimeRemaining, mixxx::Duration::Precision::CENTISECONDS));
        } else {
            setText(QLatin1String("-") % timeFormat(
                    -dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS)
                    % QLatin1String("  -") % timeFormat(
                    dTimeRemaining, mixxx::Duration::Precision::CENTISECONDS));
        }
    }
    m_dOldTimeElapsed = dTimeElapsed;
}

// m_pTimeElapsed is not updated when paused at the beginning of a track,
// but m_pTimeRemaining is updated in that case. So, call slotSetTimeElapsed to
// update this widget's text.
void WNumberPos::slotTimeRemainingUpdated(double dTimeRemaining) {
    Q_UNUSED(dTimeRemaining);
    double dTimeElapsed = m_pTimeElapsed->get();
    if (dTimeElapsed == 0.0) {
        slotSetTimeElapsed(dTimeElapsed);
    }
}

void WNumberPos::slotSetDisplayMode(double remain) {
    if (remain == 1.0) {
        m_displayMode = TrackTime::DisplayMode::Remaining;
    } else if (remain == 2.0) {
        m_displayMode = TrackTime::DisplayMode::ElapsedAndRemaining;
    } else {
        m_displayMode = TrackTime::DisplayMode::Elapsed;
    }

    slotSetTimeElapsed(m_dOldTimeElapsed);
}
void WNumberPos::slotSetTimeFormat(double v) {
    m_displayFormat = static_cast<TrackTime::DisplayFormat>(static_cast<int>(v));

    slotSetTimeElapsed(m_dOldTimeElapsed);
}
