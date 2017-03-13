#include <QPainter>
#include <QtDebug>

#include "waveformmarkrange.h"

#include "waveformsignalcolors.h"
#include "controlobject.h"
#include "controlobjectthread.h"
#include "widget/wskincolor.h"

WaveformMarkRange::WaveformMarkRange()
        : m_markStartPointControl(NULL),
          m_markEndPointControl(NULL),
          m_markEnabledControl(NULL) {
}

WaveformMarkRange::~WaveformMarkRange() {
    delete m_markStartPointControl;
    delete m_markEndPointControl;
    delete m_markEnabledControl;
}

bool WaveformMarkRange::active() {
    const double startValue = start();
    const double endValue = end();
    return startValue != endValue && startValue != -1.0 && endValue != -1.0;
}

bool WaveformMarkRange::enabled() {
    // Default to enabled if there is no enabled control.
    return !m_markEnabledControl || !m_markEnabledControl->valid() ||
            m_markEnabledControl->get() > 0.0;
}

double WaveformMarkRange::start() {
    double start = -1.0;
    if (m_markStartPointControl && m_markStartPointControl->valid()) {
        start = m_markStartPointControl->get();
    }
    return start;
}

double WaveformMarkRange::end() {
    double end = -1.0;
    if (m_markEndPointControl && m_markEndPointControl->valid()) {
        end = m_markEndPointControl->get();
    }
    return end;
}

void WaveformMarkRange::setup(const QString& group, const QDomNode& node,
                              const SkinContext& context,
                              const WaveformSignalColors& signalColors) {
    m_activeColor = context.selectString(node, "Color");
    if (!m_activeColor.isValid()) {
        //vRince kind of legacy fallback ...
        // As a fallback, grab the mark color from the parent's MarkerColor
        m_activeColor = signalColors.getAxesColor();
        qDebug() << "Didn't get mark Color, using parent's <AxesColor>:" << m_activeColor;
    } else {
        m_activeColor = WSkinColor::getCorrectColor(m_activeColor);
    }

    m_disabledColor = context.selectString(node, "DisabledColor");
    if (!m_disabledColor.isValid()) {
        //vRince kind of legacy fallback ...
        // Read the text color, otherwise use the parent's SignalColor.
        m_disabledColor = signalColors.getSignalColor();
        qDebug() << "Didn't get mark TextColor, using parent's <SignalColor>:" << m_disabledColor;
    }

    QString startControl = context.selectString(node, "StartControl");
    if (!startControl.isEmpty()) {
        m_markStartPointControl = new ControlObjectThread(group, startControl);
    }
    QString endControl = context.selectString(node, "EndControl");
    if (!endControl.isEmpty()) {
        m_markEndPointControl = new ControlObjectThread(
                group, endControl);
    }
    QString enabledControl = context.selectString(node, "EnabledControl");
    if (!enabledControl.isEmpty()) {
        m_markEnabledControl = new ControlObjectThread(
                group, enabledControl);
    }
}

void WaveformMarkRange::generateImage(int weidth, int height) {
    m_activeImage = QImage(weidth, height, QImage::Format_ARGB32_Premultiplied);
    m_disabledImage = QImage(weidth, height, QImage::Format_ARGB32_Premultiplied);

    // fill needed cause they remain transparent
    m_activeImage.fill(QColor(0,0,0,0).rgba());
    m_disabledImage.fill(QColor(0,0,0,0).rgba());

    QColor activeColor = m_activeColor;
    activeColor.setAlphaF(0.3);
    QBrush brush(activeColor);

    QPainter painter;
    painter.begin(&m_activeImage);
    painter.fillRect(m_activeImage.rect(), brush);
    painter.end();

    QColor disabledColor = m_disabledColor;
    disabledColor.setAlphaF(0.3);
    brush = QBrush(disabledColor);

    painter.begin(&m_disabledImage);
    painter.fillRect(m_disabledImage.rect(), brush);
    painter.end();
}
