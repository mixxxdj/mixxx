// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QTime>
#include <math.h>

#include "wnumberpos.h"
#include "mathstuff.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"
#include "controlobjectthreadmain.h"

WNumberPos::WNumberPos(const char* group, QWidget* parent)
        : WNumber(parent),
          m_dOldValue(0.0f),
          m_dTrackSamples(0.0),
          m_dTrackSampleRate(0.0f),
          m_bRemain(false) {
    m_qsText = "";

    m_pShowTrackTimeRemaining = new ControlObjectThreadMain(
            "[Controls]", "ShowDurationRemaining");
    connect(m_pShowTrackTimeRemaining, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRemain(double)));
    slotSetRemain(m_pShowTrackTimeRemaining->get());

    // We cannot use the parameter from the skin because this would be a connection
    // to a ControlObjectThreadWidget and would be normalized to midi values
    // -0.14 .. 1.14.  Instead we use the engine's playposition value
    // which is normalized from 0 to 1.  As a result, the
    // <Connection> parameter is no longer necessary in skin definitions, but
    // leaving it in is harmless.
    m_pVisualPlaypos = new ControlObjectThreadMain(group, "playposition");
    connect(m_pVisualPlaypos, SIGNAL(valueChanged(double)), this, SLOT(slotSetValue(double)));

    m_pTrackSamples = new ControlObjectThreadWidget(
            group, "track_samples");
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackSamples(double)));
    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSamples->emitValueChanged();

    m_pTrackSampleRate = new ControlObjectThreadWidget(
            group, "track_samplerate");
    connect(m_pTrackSampleRate, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackSampleRate(double)));
    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSampleRate->emitValueChanged();
}

WNumberPos::~WNumberPos() {
    delete m_pTrackSampleRate;
    delete m_pTrackSamples;
    delete m_pShowTrackTimeRemaining;
}

void WNumberPos::mousePressEvent(QMouseEvent* pEvent) {
    bool leftClick = pEvent->buttons() & Qt::LeftButton;

    if (leftClick) {
        setRemain(!m_bRemain);
        m_pShowTrackTimeRemaining->slotSet(m_bRemain ? 1.0f : 0.0f);
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
}

void WNumberPos::slotSetValue(double dValue) {
    m_dOldValue = dValue;

    double valueMillis = 0.0f;
    double durationMillis = 0.0f;
    if (m_dTrackSamples > 0 && m_dTrackSampleRate > 0) {
        double dDuration = m_dTrackSamples / m_dTrackSampleRate / 2.0;
        valueMillis = dValue * 500.0f * m_dTrackSamples / m_dTrackSampleRate;
        durationMillis = dDuration * 1000.0f;
        if (m_bRemain)
            valueMillis = math_max(durationMillis - valueMillis, 0.0f);
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

    m_pLabel->setText(QString("%1%2").arg(m_qsText, valueString));
}

void WNumberPos::slotSetRemain(double remain) {
    setRemain(remain > 0.0f);
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
