// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QStringBuilder>

#include "widget/wnumberpos.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"
#include "util/duration.h"

WNumberPos::WNumberPos(const char* group, QWidget* parent)
        : WNumber(parent),
          m_dOldPosition(0.0),
          m_dTrackSamples(0.0),
          m_dTrackSampleRate(0.0),
          m_bRemain(false) {
    m_pShowTrackTimeRemaining = new ControlProxy(
            "[Controls]", "ShowDurationRemaining", this);
    m_pShowTrackTimeRemaining->connectValueChanged(
            SLOT(slotSetDisplayMode(double)));
    slotSetDisplayMode(m_pShowTrackTimeRemaining->get());

    // We use the engine's playposition value directly because the parameter
    // normalization done by the widget system used to be unusable for this
    // because the range of playposition was -0.14 to 1.14 in 1.11.x. As a
    // result, the <Connection> parameter is no longer necessary in skin
    // definitions, but leaving it in is harmless.
    m_pVisualPlaypos = new ControlProxy(group, "playposition", this);
    m_pVisualPlaypos->connectValueChanged(SLOT(slotSetPosition(double)));

    m_pTrackSamples = new ControlProxy(
            group, "track_samples", this);
    m_pTrackSamples->connectValueChanged(SLOT(slotSetTrackSamples(double)));

    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSamples->emitValueChanged();

    m_pTrackSampleRate = new ControlProxy(
            group, "track_samplerate", this);
    m_pTrackSampleRate->connectValueChanged(
            SLOT(slotSetTrackSampleRate(double)));

    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSampleRate->emitValueChanged();

    slotSetPosition(m_pVisualPlaypos->get());
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
        slotSetPosition(m_dOldPosition);
    }
}

void WNumberPos::slotSetTrackSamples(double dSamples) {
    m_dTrackSamples = dSamples;
    slotSetPosition(m_dOldPosition);
}

void WNumberPos::slotSetTrackSampleRate(double dSampleRate) {
    m_dTrackSampleRate = dSampleRate;
    slotSetPosition(m_dOldPosition);
}

// Reimplementing WNumber::setValue
void WNumberPos::setValue(double dValue) {
    // Ignore midi-scaled signals from the skin connection.
    Q_UNUSED(dValue);
    // Update our value with the old value.
    slotSetPosition(m_dOldPosition);
}

void WNumberPos::slotSetPosition(double dPosition) {
    m_dOldPosition = dPosition;

    double dPosSecondsElapsed = 0.0;
    double dPosSecondsRemaining = 0.0;
    if (m_dTrackSamples > 0 && m_dTrackSampleRate > 0) {
        double dDurationSeconds = (m_dTrackSamples / 2.0) / m_dTrackSampleRate;
        double dDurationMillis = dDurationSeconds * 1000.0;
        double dPosMillis = dPosition * dDurationMillis;
        dPosSecondsElapsed = dPosMillis / 1000.0;
        if (m_displayMode != TrackTime::DisplayMode::Elapsed) {
            double dPosMillisRemaining = math_max(dDurationMillis - dPosMillis, 0.0);
            dPosSecondsRemaining = dPosMillisRemaining / 1000.0;
        }
    }

    if (m_displayMode == TrackTime::DisplayMode::Elapsed) {
        if (dPosSecondsElapsed >= 0.0) {
            setText(mixxx::Duration::formatSeconds(
                        dPosSecondsElapsed, mixxx::Duration::Precision::CENTISECONDS));
        } else {
            setText(QLatin1String("-") % mixxx::Duration::formatSeconds(
                        -dPosSecondsElapsed, mixxx::Duration::Precision::CENTISECONDS));
        }
    } else if (m_displayMode == TrackTime::DisplayMode::Remaining) {
        setText(QLatin1String("-") % mixxx::Duration::formatSeconds(
                    dPosSecondsRemaining, mixxx::Duration::Precision::CENTISECONDS));
    } else if (m_displayMode == TrackTime::DisplayMode::ElapsedAndRemaining) {
        if (dPosSecondsElapsed >= 0.0) {
            setText(mixxx::Duration::formatSeconds(
                        dPosSecondsElapsed, mixxx::Duration::Precision::CENTISECONDS)
                    % QLatin1String("  -") %
                    mixxx::Duration::formatSeconds(
                        dPosSecondsRemaining, mixxx::Duration::Precision::CENTISECONDS));
        } else {
            setText(QLatin1String("-") % mixxx::Duration::formatSeconds(
                        -dPosSecondsElapsed, mixxx::Duration::Precision::CENTISECONDS)
                    % QLatin1String("  -") %
                    mixxx::Duration::formatSeconds(
                        dPosSecondsRemaining, mixxx::Duration::Precision::CENTISECONDS));
        }
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

    slotSetPosition(m_dOldPosition);
}
