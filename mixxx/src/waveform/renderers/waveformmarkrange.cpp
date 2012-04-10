#include "waveformmarkrange.h"
#include "controlobject.h"
#include "widget/wwidget.h"

#include <QPainter>
#include <QDebug>

WaveformMarkRange::WaveformMarkRange(){
    m_markStartPointControl = 0;
    m_markEndPointControl = 0;
    m_markEnabledControl = 0;
}

void WaveformMarkRange::setup( const QString& group, const QDomNode& node) {
    m_activeColor = WWidget::selectNodeQString(node, "Color");
    if (m_activeColor == "") {
        //vRince kind of legacy fallback ...
        // As a fallback, grab the mark color from the parent's MarkerColor
        m_activeColor = WWidget::selectNodeQString(node.parentNode(), "MarkerColor");
        qDebug() << "Didn't get mark Color, using parent's MarkerColor:" << m_activeColor;
    }

    m_disabledColor = WWidget::selectNodeQString(node, "DisabledColor");
    if (m_disabledColor == "") {
        //vRince kind of legacy fallback ...
        // Read the text color, otherwise use the parent's SignalColor.
        m_disabledColor = WWidget::selectNodeQString(node.parentNode(), "SignalColor");
        qDebug() << "Didn't get mark TextColor, using parent's SignalColor:" << m_disabledColor;
    }

    m_markStartPointControl = ControlObject::getControl(
                ConfigKey(group, WWidget::selectNodeQString(node, "StartControl")));
    m_markEndPointControl = ControlObject::getControl(
                ConfigKey(group, WWidget::selectNodeQString(node, "EndControl")));
    m_markEnabledControl = ControlObject::getControl(
                ConfigKey(group, WWidget::selectNodeQString(node, "EnabledControl")));
}

void WaveformMarkRange::generatePixmap(int weidth, int height) {
    m_activePixmap = QPixmap(weidth, height);
    m_disabledPixmap = QPixmap(weidth, height);

    //fill needed cause they remain transparent
    m_activePixmap.fill(QColor(0,0,0,0));
    m_disabledPixmap.fill(QColor(0,0,0,0));

    QColor activeColor = m_activeColor;
    activeColor.setAlphaF(0.3);
    QBrush brush(activeColor);

    QPainter painter;
    painter.begin(&m_activePixmap);
    painter.fillRect(m_activePixmap.rect(), brush);
    painter.end();

    QColor disabledColor = m_disabledColor;
    disabledColor.setAlphaF(0.3);
    brush = QBrush(disabledColor);

    painter.begin(&m_disabledPixmap);
    painter.fillRect(m_disabledPixmap.rect(), brush);
    painter.end();
}
