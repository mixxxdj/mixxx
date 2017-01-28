// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QStringBuilder>

#include "widget/wnumberpos.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "util/math.h"
#include "util/duration.h"

WNumberPos::WNumberPos(const char* group, QWidget* parent)
        : WNumber(parent),
          m_dOldValue(0.0),
          m_dTrackSamples(0.0),
          m_dTrackSampleRate(0.0),
          m_bRemain(false) {
    m_pShowTrackTimeRemaining = new ControlProxy(
            "[Controls]", "ShowDurationRemaining", this);
    m_pShowTrackTimeRemaining->connectValueChanged(
            SLOT(slotSetRemain(double)));
    slotSetRemain(m_pShowTrackTimeRemaining->get());

    // We use the engine's playposition value directly because the parameter
    // normalization done by the widget system used to be unusable for this
    // because the range of playposition was -0.14 to 1.14 in 1.11.x. As a
    // result, the <Connection> parameter is no longer necessary in skin
    // definitions, but leaving it in is harmless.
    m_pVisualPlaypos = new ControlProxy(group, "playposition", this);
    m_pVisualPlaypos->connectValueChanged(SLOT(slotSetValue(double)));

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

    slotSetValue(m_pVisualPlaypos->get());
}

void WNumberPos::mousePressEvent(QMouseEvent* pEvent) {
    bool leftClick = pEvent->buttons() & Qt::LeftButton;

    if (leftClick) {
        setRemain(!m_bRemain);
        m_pShowTrackTimeRemaining->set(m_bRemain ? 1.0 : 0.0);
    }
}

void WNumberPos::slotSetTrackSamples(double dSamples) {
    m_dTrackSamples = dSamples;
    slotSetValue(m_dOldValue);
}

void WNumberPos::slotSetTrackSampleRate(double dSampleRate) {
    m_dTrackSampleRate = dSampleRate;
    slotSetValue(m_dOldValue);
}

void WNumberPos::setValue(double dValue) {
    // Ignore midi-scaled signals from the skin connection.
    Q_UNUSED(dValue);
    // Update our value with the old value.
    slotSetValue(m_dOldValue);
}

void WNumberPos::slotSetValue(double dValue) {
    m_dOldValue = dValue;

    double dPosSeconds = 0.0;
    if (m_dTrackSamples > 0 && m_dTrackSampleRate > 0) {
        double dDurationSeconds = (m_dTrackSamples / 2.0) / m_dTrackSampleRate;
        double dDurationMillis = dDurationSeconds * 1000.0;
        double dPosMillis = dValue * dDurationMillis;
        if (m_bRemain) {
            dPosMillis = math_max(dDurationMillis - dPosMillis, 0.0);
        }
        dPosSeconds = dPosMillis / 1000.0;
    }

    QString sPosText;
    if (dPosSeconds >= 0.0) {
        sPosText = m_skinText % mixxx::Duration::formatSeconds(
                dPosSeconds, mixxx::Duration::Precision::CENTISECONDS);
    } else {
        sPosText = m_skinText % QLatin1String("-") % mixxx::Duration::formatSeconds(
                -dPosSeconds, mixxx::Duration::Precision::CENTISECONDS);
    }
    setText(sPosText);
}

void WNumberPos::slotSetRemain(double remain) {
    setRemain(remain > 0.0);
}

void WNumberPos::setRemain(bool bRemain) {
    m_bRemain = bRemain;

    // Shift display state between showing position and remaining
    if (m_bRemain) {
        m_skinText = "-";
    } else {
        m_skinText = "";
    }
    // Have the widget redraw itself with its current value.
    slotSetValue(m_dOldValue);
}
