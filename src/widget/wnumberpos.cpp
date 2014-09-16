// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QTime>

#include "wnumberpos.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "util/math.h"

WNumberPos::WNumberPos(const char* group, QWidget* parent)
        : WNumber(parent),
          m_dOldValue(0.0),
          m_dTrackSamples(0.0),
          m_dTrackSampleRate(0.0),
          m_bRemain(false) {
    m_qsText = "";

    m_pShowTrackTimeRemaining = new ControlObjectThread(
            "[Controls]", "ShowDurationRemaining");
    m_pShowTrackTimeRemaining->connectValueChanged(
            this, SLOT(slotSetRemain(double)));
    slotSetRemain(m_pShowTrackTimeRemaining->get());

    // We use the engine's playposition value directly because the parameter
    // normalization done by the widget system used to be unusable for this
    // because the range of playposition was -0.14 to 1.14 in 1.11.x. As a
    // result, the <Connection> parameter is no longer necessary in skin
    // definitions, but leaving it in is harmless.
    m_pVisualPlaypos = new ControlObjectThread(group, "playposition");
    m_pVisualPlaypos->connectValueChanged(this, SLOT(slotSetValue(double)));

    m_pTrackSamples = new ControlObjectThread(
            group, "track_samples");
    m_pTrackSamples->connectValueChanged(
            this, SLOT(slotSetTrackSamples(double)));

    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSamples->emitValueChanged();

    m_pTrackSampleRate = new ControlObjectThread(
            group, "track_samplerate");
    m_pTrackSampleRate->connectValueChanged(
            this, SLOT(slotSetTrackSampleRate(double)));

    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSampleRate->emitValueChanged();

    slotSetValue(m_pVisualPlaypos->get());
}

WNumberPos::~WNumberPos() {
    delete m_pTrackSampleRate;
    delete m_pTrackSamples;
    delete m_pVisualPlaypos;
    delete m_pShowTrackTimeRemaining;
}

void WNumberPos::mousePressEvent(QMouseEvent* pEvent) {
    bool leftClick = pEvent->buttons() & Qt::LeftButton;

    if (leftClick) {
        setRemain(!m_bRemain);
        m_pShowTrackTimeRemaining->slotSet(m_bRemain ? 1.0 : 0.0);
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
    double durationMillis = 0.0;
    if (m_dTrackSamples > 0 && m_dTrackSampleRate > 0) {
        double dDuration = m_dTrackSamples / m_dTrackSampleRate / 2.0;
        valueMillis = dValue * 500.0 * m_dTrackSamples / m_dTrackSampleRate;
        durationMillis = dDuration * 1000.0;
        if (m_bRemain)
            valueMillis = math_max(durationMillis - valueMillis, 0.0);
    }

    QString valueString;
    if (valueMillis >= 0) {
        QTime valueTime = QTime().addMSecs(valueMillis);
        valueString = valueTime.toString((valueTime.hour() >= 1) ? "hh:mm:ss.zzz" : "mm:ss.zzz");
    } else {
        QTime valueTime = QTime().addMSecs(0 - valueMillis);
        valueString = valueTime.toString((valueTime.hour() >= 1) ? "-hh:mm:ss.zzz" : "-mm:ss.zzz");
    }

    // The format string gives us one extra digit of millisecond precision than
    // we care about. Slice it off.
    valueString = valueString.left(valueString.length() - 1);

    setText(QString("%1%2").arg(m_qsText, valueString));
}

void WNumberPos::slotSetRemain(double remain) {
    setRemain(remain > 0.0);
}

void WNumberPos::setRemain(bool bRemain)
{
    m_bRemain = bRemain;

    // Shift display state between showing position and remaining
    if (m_bRemain)
        m_qsText = "-";
    else
        m_qsText = "";

    // Have the widget redraw itself with its current value.
    slotSetValue(m_dOldValue);
}
