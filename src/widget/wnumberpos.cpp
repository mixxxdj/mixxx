// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QStringBuilder>

#include "widget/wnumberpos.h"
#include "controlobject.h"
#include "controlobjectslave.h"
#include "util/math.h"
#include "util/time.h"

WNumberPos::WNumberPos(const char* group, QWidget* parent)
        : WNumber(parent),
          m_dOldValue(0.0),
          m_dTrackSamples(0.0),
          m_dTrackSampleRate(0.0),
          m_bRemain(false) {
    m_pShowTrackTimeRemaining = new ControlObjectSlave(
            "[Controls]", "ShowDurationRemaining", this);
    m_pShowTrackTimeRemaining->connectValueChanged(
            SLOT(slotSetRemain(double)));
    slotSetRemain(m_pShowTrackTimeRemaining->get());

    // We use the engine's playposition value directly because the parameter
    // normalization done by the widget system used to be unusable for this
    // because the range of playposition was -0.14 to 1.14 in 1.11.x. As a
    // result, the <Connection> parameter is no longer necessary in skin
    // definitions, but leaving it in is harmless.
    m_pVisualPlaypos = new ControlObjectSlave(group, "playposition", this);
    m_pVisualPlaypos->connectValueChanged(SLOT(slotSetValue(double)));

    m_pTrackSamples = new ControlObjectSlave(
            group, "track_samples", this);
    m_pTrackSamples->connectValueChanged(SLOT(slotSetTrackSamples(double)));

    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSamples->emitValueChanged();

    m_pTrackSampleRate = new ControlObjectSlave(
            group, "track_samplerate", this);
    m_pTrackSampleRate->connectValueChanged(
            SLOT(slotSetTrackSampleRate(double)));

    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSampleRate->emitValueChanged();

    slotSetValue(m_pVisualPlaypos->get());
}

WNumberPos::~WNumberPos() {
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

    double valueMillis = 0.0;
    if (m_dTrackSamples > 0 && m_dTrackSampleRate > 0) {
        double dDuration = m_dTrackSamples / m_dTrackSampleRate / 2.0;
        valueMillis = dValue * 500.0 * m_dTrackSamples / m_dTrackSampleRate;
        double durationMillis = dDuration * Time::kMillisPerSecond;
        if (m_bRemain)
            valueMillis = math_max(durationMillis - valueMillis, 0.0);
    }

    QString valueString;
    if (valueMillis >= 0) {
        valueString = m_skinText % Time::formatSeconds(
                valueMillis / Time::kMillisPerSecond, Time::Precision::CENTISECONDS);
    } else {
        valueString = m_skinText % QLatin1String("-") % Time::formatSeconds(
                -valueMillis / Time::kMillisPerSecond, Time::Precision::CENTISECONDS);
    }
    setText(valueString);
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
