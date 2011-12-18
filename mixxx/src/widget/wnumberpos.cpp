// Tue Haste Andersen <haste@diku.dk>, (C) 2003

#include <QTime>
#include <math.h>

#include "wnumberpos.h"
#include "mathstuff.h"
#include "controlobject.h"
#include "controlobjectthreadwidget.h"
#include "controlobjectthreadmain.h"

WNumberPos::WNumberPos(const char * group, QWidget * parent)
        : WNumber(parent),
          m_dOldValue(0.0f),
          m_dTrackSamples(0.0),
          m_dTrackSampleRate(0.0f),
          m_bRemain(false) {
    m_qsText = "";

    m_pShowDurationRemaining = new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Controls]", "ShowDurationRemaining")));
    connect(m_pShowDurationRemaining, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetRemain(double)));
    slotSetRemain(m_pShowDurationRemaining->get());

    // TODO(xxx) possible unused m_pRateControl and m_pRateDirControl?
    m_pRateControl = new ControlObjectThreadWidget(
        ControlObject::getControl(ConfigKey(group, "rate")));
    m_pRateDirControl = new ControlObjectThreadWidget(
        ControlObject::getControl(ConfigKey(group, "rate_dir")));

    m_pTrackSamples = new ControlObjectThreadWidget(
        ControlObject::getControl(ConfigKey(group, "track_samples")));
    connect(m_pTrackSamples, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackSamples(double)));
    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSamples->emitValueChanged();

    m_pTrackSampleRate = new ControlObjectThreadWidget(
        ControlObject::getControl(ConfigKey(group, "track_samplerate")));
    connect(m_pTrackSampleRate, SIGNAL(valueChanged(double)),
            this, SLOT(slotSetTrackSampleRate(double)));
    // Tell the CO to re-emit its value since we could be created after it was
    // set to a valid value.
    m_pTrackSampleRate->emitValueChanged();
}

WNumberPos::~WNumberPos() {
    delete m_pTrackSampleRate;
    delete m_pTrackSamples;
    delete m_pShowDurationRemaining;
    delete m_pRateControl;
    delete m_pRateDirControl;
}

void WNumberPos::mousePressEvent(QMouseEvent* pEvent) {
    bool leftClick = pEvent->buttons() & Qt::LeftButton;

    if (leftClick) {
        setRemain(!m_bRemain);
        m_pShowDurationRemaining->slotSet(m_bRemain ? 1.0f : 0.0f);
    }
}

void WNumberPos::slotSetTrackSamples(double dSamples) {
    m_dTrackSamples = dSamples;
    setValue(m_dOldValue);
}

void WNumberPos::slotSetTrackSampleRate(double dSampleRate) {
    m_dTrackSampleRate = dSampleRate;
    setValue(m_dOldValue);
}

void WNumberPos::setValue(double dValue) {
    m_dOldValue = dValue;

    double valueMillis = 0.0f;
    double durationMillis = 0.0f;
    if (m_dTrackSamples > 0 && m_dTrackSampleRate > 0) {
        //map midi value taking in to account 14 = 0 and 114 = 1
        double dDuration = m_dTrackSamples / m_dTrackSampleRate / 2.0;
        valueMillis = (dValue - 14) * 1000.0f * m_dTrackSamples / 2.0f / 100.0f / m_dTrackSampleRate;
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
    setValue(m_dOldValue);
}






