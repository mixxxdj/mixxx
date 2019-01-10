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
    m_pTimeElapsed->connectValueChanged(this, &WNumberPos::slotSetTimeElapsed);
    m_pTimeRemaining = new ControlProxy(group, "time_remaining", this);
    m_pTimeRemaining->connectValueChanged(this, &WNumberPos::slotTimeRemainingUpdated);

    m_pShowTrackTimeRemaining = new ControlProxy(
            "[Controls]", "ShowDurationRemaining", this);
    m_pShowTrackTimeRemaining->connectValueChanged(this, &WNumberPos::slotSetDisplayMode);
    slotSetDisplayMode(m_pShowTrackTimeRemaining->get());
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

    if (m_displayMode == TrackTime::DisplayMode::Elapsed) {
        if (dTimeElapsed >= 0.0) {
            setText(mixxx::Duration::formatSeconds(
                        dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS));
        } else {
            setText(QLatin1String("-") % mixxx::Duration::formatSeconds(
                        -dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS));
        }
    } else if (m_displayMode == TrackTime::DisplayMode::Remaining) {
        setText(QLatin1String("-") % mixxx::Duration::formatSeconds(
                    dTimeRemaining, mixxx::Duration::Precision::CENTISECONDS));
    } else if (m_displayMode == TrackTime::DisplayMode::ElapsedAndRemaining) {
        if (dTimeElapsed >= 0.0) {
            setText(mixxx::Duration::formatSeconds(
                        dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS)
                    % QLatin1String("  -") %
                    mixxx::Duration::formatSeconds(
                        dTimeRemaining, mixxx::Duration::Precision::CENTISECONDS));
        } else {
            setText(QLatin1String("-") % mixxx::Duration::formatSeconds(
                        -dTimeElapsed, mixxx::Duration::Precision::CENTISECONDS)
                    % QLatin1String("  -") %
                    mixxx::Duration::formatSeconds(
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
